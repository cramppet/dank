# dank: a deterministic finite automata ranker 

The `dank` package implements an algorithm called **ranking** for finite regular
languages; **ranking** forms a bijection between the elements of the regular
language (strings) and ordinal values (integers); ordinal values represent a
string's position within the lexicographically ordered set of all strings of the
language. Since this kind of ranking operates on regular languages, we can use
**regular expressions** as a compact representation of the language.

I can see this package as having several main use cases, though there are
probably more:

1.  **Programmable encoders** for the serialization of arbitrary data into some
    format modeled as a regular language.
2.  **Programmable generators** for data modeled as a regular language.
3.  **Optimal compression** for data modeled as a regular language.

Some possible applications include:

- Fuzzing/brute force/dictionary generation:
    - DNS, password, web fuzzing, etc.
- Network obfuscation:
    - C2 redirector that generates responses using programmatic encodings
    - C2 traffic obfuscation (generate MalleableC2 configuration blocks)
- Payload obfuscation
    - MalleableC2 (see above)
- Compression

# Install

If you are on a modern version of Linux, you should be able to run:

```
python3 -m pip install --user dank
```

Hopefully that just works. If it doesn't, open an issue. Other platforms aren't
explicitly supported at this time, but Windows can be accommodated with WSL.

# Examples

## Warmup

Here's a quick overview with the simplest possible usage:

```python
>>> import dank
>>> encoder = dank.DFAEncoder('(a|b)+', 6)
>>> encoder.unrank(42)
b'bababa'
>>> bin(42)
'0b101010'
```

What we just did was encode the value `42` into a representation described by
the regular expression `(a|b)+` with a fixed slice value of 6. A "fixed slice"
defines the length restriction imposed on the finite regular language. This
parameter controls the size of the language and consequently the largest amount
of data you can encode in a single operation.

Do you notice anything interesting in the output? The binary representation
`101010` and the encoded representation `bababa` are in fact identical, simply
replace `a` with `0` and `b` with `1`.

What this illustrates is how the process works at a basic level, it operates
a lot like a radix change, except the process generalizes to an arbitrary
regular language.

## Programmable Encoders

Let's say you want to transfer data from point A to point B using some
pre-defined format pretending to be HTTP:

```
GET (a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+.txt HTTP/1.1
Host: foobar.c2domain.com
Connection: Keep-Alive
```

This format can be used to serialize and send arbitrary data masquerading as
HTTP traffic. This will be referred to as **protocol mimicry**. Mimicry is
powerful but has limitations that must be accounted for. You must ensure that
the buffer being sent as HTTP traffic is "valid enough" for it to make it to
your destination.

This often means that you have to perform some post-processing on it, like
patching in HTTP headers for `Content-Length`. This is why you should really
only send data in the HTTP body section, not via headers. You can, but it's more
work for you recover it and not to mention, it's much less efficient.

```python
import dank

regex = b'''GET (a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+.txt HTTP/1.1
Host: foobar.c2domain.com
Connection: Keep-Alive
'''

encoder = dank.DFAEncoder(regex, 100)
data = 'Hello, world'

encoded = encoder.unrank(int.from_bytes(data.encode('ascii'), 'big'))
print(encoded)

decoded = encoder.rank(encoded).to_bytes(len(data), 'big').decode('ascii')
print(decoded)
```

Now consider the `len(data)` expression when performing decoding, this
expression can be computed in this example, but you won't be able to at runtime.
In practice you will want to create a small binary message format to ensure that
you can send/receive data, something like:

```
+-----------------+--------------------------+---------------+
| Magic (2 bytes) | Message length (4 bytes) | Message bytes |
+-----------------|--------------------------+---------------+
```

```python
import math
import struct

import dank

regex = b'''GET (a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+.txt HTTP/1.1
Host: foobar.c2domain.com
Connection: Keep-Alive
'''

class MySimpleEncoder():

    ENCODER_MAGIC = b'\xBE\xBA'

    def __init__(self, regex, fixed_slice=256):
        self.encoder = dank.DFAEncoder(regex, fixed_slice)

    def encode(self, data: bytes) -> bytes:
        data_len = len(data)
        header = struct.pack('>I', data_len)
        message = self.ENCODER_MAGIC + header + data
        encoded = self.encoder.unrank(int.from_bytes(message, 'big'))
        return encoded

    def decode(self, cover: bytes) -> bytes:
        raw = self.encoder.rank(cover)
        raw = raw.to_bytes(math.ceil(len(bin(raw))/8), 'big')
        try:
            idx = raw.index(self.ENCODER_MAGIC)
            message = raw[idx+len(self.ENCODER_MAGIC):]
            data_len = struct.unpack('>I', message[:4])[0]
            data = message[4:4+data_len]
            return data
        except ValueError:
            raise Exception('Invalid sequence provided for decoding')

encoder = MySimpleEncoder(regex)
data = 'Hello, world'.encode('ascii)

encoded = encoder.encode(data)
print(encoded)

decoded = encoder.decode(encoded)
print(decoded)
```

## Programmable Generators

Let's suppose you want to generate some content for fuzzing/brute forcing; if
you can collect data and generalize formats for what you observed, then you can
easily synthesize more test cases:

```python
import random
import dank

# Ex: prod-host1421.example.com, stg-host12414.example.com, etc.
dns_fuzz_format = b'(dev|prod|stg)-host(1|2|3|4)+.example.com'
fixed_slice = 25

encoder = dank.DFAEncoder(dns_fuzz_format, fixed_slice)
num_words = encoder.num_words(fixed_slice, fixed_slice)

# Iterative selection
for i in range(1000):
    print(encoder.unrank(i))

# Random selection
for i in range(1000):
    e = random.randint(0, num_words)
    print(encoder.unrank(e))
```

## Optimal compression

The compression use case was the original case for which Goldberg-Sipser
developed the ranking algorithm used here. Thus, it is the most natural one to
work with. The compression/decompression is achieved simply through the `rank`
and `unrank` functions respectively:

```python
>>> import dank
>>> encoder = dank.DFAEncoder('hello darkness my old (a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)+', 28)
>>> encoder.rank('hello darkness my old friend')
169363224
```

Now, the ordinal value `169,363,224` can be represented using a standard 32-bit
integer (**4 bytes**). The original string `hello darkness my old friend` took
**28 bytes** to represent. Thus, we have compressed the string's representation
by exactly **7 times**, meaning we have seven times less data to store or seven
times less data to send.

# Credits

This project is based on three separate projects:

1. https://github.com/nerddan/regex2dfa
    - Provides a simple regex engine which compiles regexes to minimal DFAs
    - [Thompson's construction algorithm](https://en.wikipedia.org/wiki/Thompson%27s_construction)
    - [Powerset construction](https://en.wikipedia.org/wiki/Powerset_construction)
    - [Brzozowski's DFA minimization algorithm](https://en.wikipedia.org/wiki/DFA_minimization#Brzozowski's_algorithm)

2. https://github.com/kpdyer/libfte
    - Provides an implementation of Goldberg-Sipser ranking
    - http://dl.acm.org/citation.cfm?id=22194
    - http://dl.acm.org/citation.cfm?id=2516657

3. https://github.com/sercantutar/infint
    - Provides an implementation of arbitrary precision integer arithmetic
    - https://en.wikipedia.org/wiki/Arbitrary-precision_arithmetic
