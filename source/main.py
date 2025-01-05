import struct

# SHA-256 constants
SHA256_BLOCK_SIZE = 64
SHA256_DIGEST_SIZE = 32

# SHA-256 context structure
class SHA256_CTX:
    def __init__(self):
        self.state = [
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        ]
        self.bit_count = 0
        self.buffer = bytearray(SHA256_BLOCK_SIZE)

# SHA-256 constants
kConstants = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
]

# Right rotation
def rotr(x, n):
    return ((x >> n) | (x << (32 - n))) & 0xFFFFFFFF

# SHA-256 helper functions
def CH(x, y, z):
    return (x & y) ^ (~x & z)

def MAJ(x, y, z):
    return (x & y) ^ (x & z) ^ (y & z)

def EP0(x):
    return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22)

def EP1(x):
    return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25)

def SIG0(x):
    return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3)

def SIG1(x):
    return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10)

# SHA-256 Transform
def sha256_transform(ctx, data):
    w = [0] * 64
    for t in range(16):
        w[t] = struct.unpack(">I", data[t * 4:t * 4 + 4])[0]
    for t in range(16, 64):
        w[t] = (SIG1(w[t - 2]) + w[t - 7] + SIG0(w[t - 15]) + w[t - 16]) & 0xFFFFFFFF

    a, b, c, d, e, f, g, h = ctx.state

    for t in range(64):
        t1 = (h + EP1(e) + CH(e, f, g) + kConstants[t] + w[t]) & 0xFFFFFFFF
        t2 = (EP0(a) + MAJ(a, b, c)) & 0xFFFFFFFF
        h = g
        g = f
        f = e
        e = (d + t1) & 0xFFFFFFFF
        d = c
        c = b
        b = a
        a = (t1 + t2) & 0xFFFFFFFF

    ctx.state = [(ctx.state[i] + v) & 0xFFFFFFFF for i, v in enumerate([a, b, c, d, e, f, g, h])]

# Initialize
def sha256_init(ctx):
    ctx.__init__()

# Update
def sha256_update(ctx, data):
    data_len = len(data)
    index = (ctx.bit_count // 8) % SHA256_BLOCK_SIZE
    ctx.bit_count += data_len * 8

    part_len = SHA256_BLOCK_SIZE - index
    if data_len >= part_len:
        ctx.buffer[index:] = data[:part_len]
        sha256_transform(ctx, ctx.buffer)
        i = part_len
        while i + SHA256_BLOCK_SIZE <= data_len:
            sha256_transform(ctx, data[i:i + SHA256_BLOCK_SIZE])
            i += SHA256_BLOCK_SIZE
        index = 0
    else:
        i = 0

    ctx.buffer[index:index + data_len - i] = data[i:]

# Finalize
def sha256_finalize(ctx):
    index = (ctx.bit_count // 8) % SHA256_BLOCK_SIZE
    pad_len = SHA256_BLOCK_SIZE - index if index < 56 else 120 - index
    padding = b"\x80" + b"\x00" * (pad_len - 1)
    length = struct.pack(">Q", ctx.bit_count)

    sha256_update(ctx, padding + length)

    return b"".join(struct.pack(">I", state) for state in ctx.state)

# Main function
def main():
    message = b"Hello, libu!"
    ctx = SHA256_CTX()

    sha256_init(ctx)
    sha256_update(ctx, message)
    hash_output = sha256_finalize(ctx)

    print("SHA-256 hash:")
    print("".join(f"{byte:02x}" for byte in hash_output))

if __name__ == "__main__":
    main()
