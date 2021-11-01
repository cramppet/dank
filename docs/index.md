# dank's documentation

Here you will find the documentation for the `dank` package created by [Peter
Crampton](https://cramppet.github.io).

## Foreword and Credits

`dank` is a self-contained and highly portable package which implements all of
the necessary components to make use of an algorithm called "Goldberg-Sipser
ranking". This algorithm was originally published in 1991 by [Michael
Sipser](https://en.wikipedia.org/wiki/Michael_Sipser) and [Andrew
Goldberg](https://en.wikipedia.org/wiki/Andrew_V._Goldberg). You can find their
original paper called "Compression and Ranking" here:
   - [https://doi.org/10.1137/0220034](https://doi.org/10.1137/0220034)

This work was further based on the work described by [Kevin
Dyer](https://kpdyer.com/) et. al. within the paper "Protocol misidentification
made easy with format-transforming encryption":
   - [https://doi.org/10.1145/2508859.2516657](https://doi.org/10.1145/2508859.2516657)

I also used the `regex2dfa` project found here for the foundational regular
expression engine with some minor modifications:
   - [https://github.com/nerddan/regex2dfa](https://github.com/nerddan/regex2dfa).

Finally, the `InfInt` arbitrary precision arithmetic library was used to support
the necessary numerical computation within the native Python extension:
   - [https://github.com/sercantutar/infint](https://github.com/sercantutar/infint)

This project would not be possible without the contributions of these authors.

## Introduction

As explained above, this package implements an algorithm called Goldberg-Sipser
ranking or just "ranking" in this context. Ranking is a process whereby we
associate an ordinal value (based on a lexicographical ordering) with the
elements of some finite regular language. In other words, this algorithm forms a
**bijection** between the integers and elements of a regular language.

The effect of this algorithm is that we can use compact descriptions of regular
languages called **regular expressions** (regexes) to create **programmatic
encoders**. That is, encoders whose operation is changeable in real-time and
which can generate output describable by any regex.

The applications of this algorithm are numerous. One notable use case was
described by Kevin Dyer et. al. in the paper above where they used ranking to
implement a cryptographic primitive called ["Format Transforming Encryption"
(FTE)](https://en.wikipedia.org/wiki/Format-transforming_encryption). The work
demonstrated the efficacy of using ranking as a means to bypass network traffic
analysis systems circa. 2012-2013.

The `dank` package was meant to address the major shortcoming with using the
ranking algorithm which is that it is relatively obscure and had no simplified
interface for building applications. All existing tooling at the time of writing
either relied upon packages that are now broken/deprecated or they did not allow
for general purpose work to be done as they were tethered to a specific use
case/platform/operating system.

`dank` runs across every major platform including GNU/Linux, Windows and macOS.
Moreover, it can be run on any architecture with a `c++11` compatible compiler.

## Installation

The following command should work on practically any system. If you run into
problems, you are encouraged to submit a bug report on the project's [GitHub
page](https://github.com/cramppet/dank).

```
python3 -m pip install --upgrade dank
```

## Regular Expression Syntax

The `dank` package implements a simple subset of regular expression syntax which
is widely used.

The syntax **DOES** support the following subset of features:

- Simple grouping using `()` syntax
- Greedy matching operators: `+`, `?`, `*`, `{n}`, `{n,}`, `{n,m}`
- Simple character classes: `[a-z]`
   - We do not support syntax like `[a-zA-Z0-9]` instead you must use
     `([a-z]|[A-Z]|[0-9])`

The syntax does **DOES NOT** support any other common operations such as:

- Wildcard character `.`
- Backreferences `\1`, ...
- Escape sequences such as `\s`, `\w`, `\d`, ...
- Capture groups
- Anchors such as start-of-line `^` and end-of-line `$`
- Non-greedy matching `??`, `*?`, `+?`, ...
- Lookahead/lookbehind
- Trailing/inline operations
- ...

Pull requests are open for any features you want to implement. Be forewarned
that some of the features listed above will never be possible to support
because they cannot be reduced down to a proper regular language describable by
a deterministic finite automata (DFA).

## Examples

Given the limited subset of features supported, you may think that `dank` cannot
do very much, but this is not true.

The package can support arbitrarily complex regular expressions but you simply
have to be knowledgeable enough to construct a proper representation for your
problem.

The following examples are meant to show you what is possible when working
within the bounds defined above.

### IPv4 addresses

```
((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9]).){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])
```

### Universally unique identifiers (UUIDs)

```
([a-f]|[0-9]){8}-(([a-f]|[0-9]){4}-){3}([a-f]|[0-9]){12}
```


## `dank` API documentation

```{eval-rst}
.. autoclass:: dank.DankEncoder.DankEncoder
    :special-members:
    :members:
```
