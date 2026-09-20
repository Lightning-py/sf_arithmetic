"""Deterministic differential checks against Python's arbitrary-precision int."""
import random
import subprocess
import sys

if hasattr(sys, "set_int_max_str_digits"):
    sys.set_int_max_str_digits(0)

rng = random.Random(20260920)
cases = []

def quotient(a, b):
    return (abs(a) // abs(b)) * (-1 if (a < 0) != (b < 0) else 1)

def add(op, a, b, expected, rc=0):
    cases.append((op, a, b, expected, rc))

edges = [0, 1, -1, 2, -2]
for bits in [31, 32, 33, 63, 64, 65, 127, 128, 129, 159, 160, 191, 192, 223, 224, 255, 256, 511, 512]:
    for delta in [-1, 0, 1]:
        edges.extend([2**bits + delta, -(2**bits + delta)])

pairs = [(a, b) for a in edges[:23] for b in edges[:23]]
pairs += [(a, rng.choice(edges)) for a in edges]
for _ in range(350):
    a = rng.getrandbits(rng.randrange(1, 2049)) * rng.choice([-1, 1])
    b = rng.getrandbits(rng.randrange(1, 2049)) * rng.choice([-1, 1])
    pairs.append((a, b))
# Sparse low halves and odd split lengths exposed the old recursive shifts.
for limbs in [5, 6, 7, 8, 9, 15, 16, 17, 31, 32, 33, 64]:
    for low in [0, 1, 2**32 - 1]:
        a = 2**(32 * (limbs - 1)) + low
        pairs.append((a, a))
        pairs.append((a, 2**(32 * limbs) - 1))
for a, b in pairs:
    for op, value in [('add', a + b), ('sub', a - b), ('mul', a * b)]:
        for suffix in ['', 'A', 'B']:
            add(op + suffix, a, b, value)
    add('karatsuba', a, b, a * b)
    add('fft', a, b, a * b)
    for suffix in ['', 'A', 'B']:
        add('div' + suffix, a, b, quotient(a, b) if b else (a if suffix == 'A' else b if suffix == 'B' else 0), 0 if b else -2)
    add('compare', a, b, a, 0 if a == b else 1 if a > b else 2)

for a in edges + [rng.getrandbits(4096) * rng.choice([-1, 1]) for _ in range(40)]:
    add('parse', a, 0, a)
    add('text', a, 0, a)
    add('mul10', a, 0, a * 10)
    add('div10', a, 0, quotient(a, 10))
    for shift in [0, 1, 31, 32, 33, 63, 64, 65, 127, 512, 2048]:
        add('left', a, shift, a << shift)
        add('right', a, shift, quotient(a, 1 << shift))
    add('right', a, 2**64 - 1 if sys.maxsize > 2**32 else 2**32 - 1, 0)

# Knuth D: normalisation shifts 0..31, quotient estimates near the base,
# long borrow chains, exact products, and remainders immediately below b.
base = 1 << 32
for shift in range(32):
    for low in [1, base - 1, base // 2 + 1]:
        b = ((1 << (31 - shift)) << 64) + low
        for q in [1, 2, base - 1, base, base + 1, base * base - 1]:
            for rem in [0, 1, b - 1]:
                a = b * q + rem
                for sa, sb in [(1, 1), (-1, 1), (1, -1), (-1, -1)]:
                    add('div', sa * a, sb * b, sa * sb * q)
# These make the trial quotient survive D3 but require D6 add-back.
for b in [(1 << 95) + 1, (1 << 95) + (base - 1), (1 << 127) + 1]:
    for a in [base * b - 1, (base + 1) * b - 1]:
        for suffix in ['', 'A', 'B']:
            add('div' + suffix, a, b, a // b)
for _ in range(1000):
    b = rng.getrandbits(rng.randrange(33, 4097)) | 1
    q = rng.getrandbits(rng.randrange(1, 2049))
    rem = rng.randrange(b)
    add('div', b * q + rem, b, q)

# FFT and automatic dispatch, beyond the old tests' 4096-bit limit.
# All-ones operands force coefficients larger than either modulus and long carries.
for bits in [4096, 8191, 8192, 8193, 16384, 32768, 65536, 131072, 262144]:
    for a, b in [((1 << bits) - 1, (1 << bits) - 1),
                 ((1 << bits) + 1, (1 << (bits - 1)) + 1),
                 (rng.getrandbits(bits), -rng.getrandbits(bits - 3)),
                 ((1 << bits) - 1, (1 << 67) + 123)]:
        add('fft', a, b, a * b)
        for suffix in ['', 'A', 'B']:
            add('mul' + suffix, a, b, a * b)

# Hex input is a test-driver convenience, avoiding quadratic decimal parsing
# for large FFT cases; decimal conversion is exercised separately above.
def encode(value):
    if abs(value).bit_length() > 10000:
        return ('-' if value < 0 else '') + hex(abs(value))
    return str(value)
payload = ''.join(f'{op} {encode(a)} {encode(b)}\n' for op, a, b, _, _ in cases)
result = subprocess.run([sys.argv[1]], input=payload, text=True, capture_output=True, timeout=180)
if result.returncode:
    raise AssertionError(f'driver exit {result.returncode}\n{result.stderr[-12000:]}')
lines = result.stdout.splitlines()
assert len(lines) == len(cases), (len(lines), len(cases))
for case, line in zip(cases, lines):
    op, a, b, expected, rc = case
    fields = line.split()
    assert len(fields) == 5, (case, line)
    actual_rc, actual, after_a, after_b, text = fields
    assert int(actual_rc) == rc and int(actual, 16) == expected, (case, line)
    expected_a = expected if op.endswith('A') or op in ('left', 'right', 'mul10', 'div10') else a
    expected_b = expected if op.endswith('B') else abs(a) % 10 if op == 'div10' else b
    assert int(after_a, 16) == expected_a and int(after_b, 16) == expected_b, (case, line)
    if op == 'text':
        assert text == str(a), (case, line)
print(f'{len(cases)} differential cases passed (seed 20260920, FFT inputs up to 262145 bits).')
