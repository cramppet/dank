# `dank` a deterministic finite automata ranker 

The `dank` package implements an algorithm called **ranking** for finite
[regular languages](https://en.wikipedia.org/wiki/Regular_language). **Ranking**
forms a bijection between the elements of a regular language (strings) and
ordinal values (integers).

The ordinal values represent a given string's position within the
lexicographically ordered set of all strings of the language. Since ranking
operates on regular languages, we can use **regular expressions** as compact
representations of these languages.

## The TL;DR of `dank`

This package has several main use cases:

1.  **Programmable encoders** for the serialization of arbitrary data into some
    format modeled as a regular language.
2.  **Programmable generators** for data modeled as a regular language.
3.  **Optimal compression** for data modeled as a regular language.

Some select applications related to the **InfoSec** domain include:

- **Fuzzing/brute force/dictionary generation**:
    - DNS, password, web fuzzing, etc.
- **Network obfuscation**:
    - C2 redirector that generates responses using programmatic encodings
    - Generate configuration blocks for [Cobalt Strike's
      MalleableC2](https://www.cobaltstrike.com/help-malleable-c2)
- **Payload obfuscation**
    - Generate configuration blocks for [Cobalt Strike's
      MalleableC2](https://www.cobaltstrike.com/help-malleable-c2)

## Install

If you are on a modern version of GNU/Linux, you should be able to run:

```
python3 -m pip install --upgrade dank
```

Hopefully that just works. If it doesn't, open an issue. Other platforms aren't
explicitly supported at this time, but Windows can be accommodated with WSL.

## Examples

### Warmup

Here's a quick overview with the simplest possible usage:

```python
>>> from dank.DankEncoder import DankEncoder
>>> encoder = DankEncoder('(a|b)+', 6)
>>> encoder.unrank(42)
b'bababa'
>>> bin(42)
'0b101010'
```

We just encoded the value `42` into a representation described by the regular
expression `(a|b)+` with a **fixed slice** value of 6. What is a "fixed slice"?
Recall that we are working with **finite** regular languages when we perform
ranking. The **fixed slice** value defines the length of strings created by the
encoder. We use this to control otherwise infinite regular languages and make
them finite (and thus "rankable").

In the case of the regular language above `(a|b)+`, this language is infinite
because it has no upper bound on the size, the `+` operator indicates "one or
*more*" where *more* has no upper bound. Thus, by imposing a fixed slice of `6`
we force this language to be finite. Incidentally, this finite language is
equivalent to `(a|b){6}`, however, general regular languages will not be as
simple to reduce.

You will have to know the desired fixed slice at runtime and provide this value
to the encoder. Most of the time this is a non-issue as you either already know
the exact length or you can set some reasonable upper bound. The encoder
supports the ability to change the fixed slice value applied, but you can only
decrease it from the original fixed slice you set upon the encoder's
initialization:

```python
>>> from dank.DankEncoder import DankEncoder
>>> encoder = DankEncoder('(a|b)+', 6)
>>> encoder.set_fixed_slice(5)
>>> encoder.unrank(12)
b'abbaa'
>>> encoder.set_fixed_slice(6)
>>> encoder.unrank(12)
b'aabbaa'
>>> encoder.set_fixed_slice(7)
AssertionError
```

One final point, do you notice anything interesting in the original output? The
binary representation `101010` and the encoded representation `bababa` are in
fact identical, simply replace `a` with `0` and `b` with `1`.  What this
illustrates is how the process works at a basic level, it operates a lot like a
[radix
change](https://en.wikipedia.org/wiki/Positional_notation#Base_conversion),
except the process generalizes to an arbitrary regular language.

### Programmable Encoders

Let's say you want to transfer data from point A to point B using some
pre-defined format pretending to be HTTP:

```
GET [a-z]+.txt HTTP/1.1
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
only send data in the HTTP body section, not via headers or in the URI
specification. You can, but it's more work for you recover it and not to
mention, it's much less efficient.

```python
from dank.DankEncoder import DankEncoder

regex = '''GET [a-z]+.txt HTTP/1.1
Host: foobar.c2domain.com
Connection: Keep-Alive
'''

encoder = DankEncoder(regex, 100)
data = 'Hello, world'

# b'GET aaaaaaaaaaaabdgazjsbmjirhingfwqxo.txt HTTP/1.1\nHost: foobar.c2domain.com...'
encoded = encoder.unrank(int.from_bytes(data.encode('ascii'), 'big'))
print(encoded) 

# Hello, world
decoded = encoder.rank(encoded).to_bytes(len(data), 'big').decode('ascii')
print(decoded)
```

### Programmable Generators

Let's suppose you want to generate some content for fuzzing/brute forcing; if
you can collect data and generalize formats for what you observed, then you can
easily synthesize more test cases using `DankGenerator`:

```python
import random
from dank.DankGenerator import DankGenerator

dns_fuzz_format = '(dev|prd|stg)-host[1-4]+.example.com'

for dns_name in DankGenerator(dns_fuzz_format, random=True, number=10):
  print(dns_name)
```

This will produce output similar to the following:

```
b'stg-host42421.example.com'
b'dev-host33411.example.com'
b'dev-host14443.example.com'
b'dev-host42112.example.com'
b'stg-host23143.example.com'
b'prd-host31122.example.com'
b'prd-host42411.example.com'
b'prd-host13324.example.com'
b'dev-host12324.example.com'
b'prd-host32344.example.com'
```

### Optimal compression

The compression use case was the original case for which Goldberg-Sipser
developed the ranking algorithm used here. Thus, it is the most natural one to
work with. The compression/decompression is achieved simply through the `rank`
and `unrank` functions respectively:

```python
>>> from dank.DankEncoder import DankEncoder
>>> encoder = DankEncoder('hello darkness my old [a-z]+', 28)
>>> encoder.rank('hello darkness my old friend')
169363224
```

Now, the ordinal value `169,363,224` can be represented using a standard 32-bit
integer (**4 bytes**). The original string `hello darkness my old friend` took
**28 bytes** to represent in ASCII. Thus, we have compressed the string's
representation by exactly **7 times**, meaning we have seven times less data to
store/send.

## Credits

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
