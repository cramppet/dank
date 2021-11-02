#!/usr/bin/env python3

import re

try:
    from ._dank import DFA, DFAEncoder
except ImportError as ex:
    print(f'dank module failed to load: {ex}')


class DankEncoder:
    """`DankEncoder` is a simplified interface on top of the native extension's
    `DFAEncoder`. `DankEncoder` provides common syntactic sugar for regular
    expressions such as **character classes** and **repetition operations**.

    Even with these features, the syntax supported by this class is simplified
    and you should review the documentation to understand the specifics of how
    it differs from what you are use to.
    """
    def __init__(self, regex: str, fixed_slice: int):
        """Initializes a new instance of the DankEncoder given a regex and a
        finite length for the regular language called a "fixed slice".

        :param regex: the regular expression to use for encoding/decoding
        :type regex: str

        :param fixed_slice: the finite length of the regular language
        :type fixed_slice: int
        """
        self.regex = DankEncoder.preprocess(regex)
        self.max_fixed_slice = fixed_slice
        self.fixed_slice = fixed_slice
        self._encoder = DFAEncoder(self.regex, fixed_slice)

    def num_states(self) -> int:
        return self._encoder.num_states()

    def num_words(self, low: int, high: int) -> int:
        """Returns the number of elements of the regular language given an upper
        and lower bound on the finite length. Usually these bounds are the same
        and correspond exactly to the value provided during initialization.

        :param low: the lower bound for calculating language members
        :type low: int

        :param high: the upper bound for calculating language members
        :type high: int

        :return: the number of elements bounded by [low, high] in the language
        :rtype: int
        """
        assert low > 0 and low <= high
        assert high > 0 and high <= self.max_fixed_slice
        return self._encoder.num_words(low, high)

    def set_fixed_slice(self, n: int):
        """Sets the value of the fixed slice for the encoder. You can use any
        fixed slice value less than the maximum one configured during
        initialization of the encoder.

        :param n: the fixed slice value to use
        :type n: int
        """
        assert n <= self.max_fixed_slice
        self.fixed_slice = n
        self._encoder.set_fixed_slice(n)

    def get_fixed_slice(self) -> int:
        """Returns the currently configured fixed slice value for the encoder.

        :return: the fixed slice currently applied
        :rtype: int
        """
        return self._encoder.get_fixed_slice()

    def unrank(self, n: int) -> bytes:
        """Performs an encoding for a given a numerical value. The number is
        transformed into a byte sequence corresponding to the n-th
        lexicographically ordered member of the finite regular language.

        :param n: the ordinal value to unrank
        :type n: int

        :return: the byte sequence corresponding to n
        :rtype: bytes
        """
        assert n < self.num_words(self.max_fixed_slice, self.max_fixed_slice)
        return self._encoder.unrank(n)

    def rank(self, s: bytes) -> int:
        """Performs a decoding of a given byte sequence into its ordinal
        representation within the finite regular language.

        :param s: the byte sequence to decode
        :type s: bytes

        :return: the ordinal value representing the byte sequence
        :rtype: int
        """
        assert len(s) == self.fixed_slice
        return self._encoder.rank(s)

    @staticmethod
    def range_expand(range_expr: str) -> str:
        """Expands a simple range syntax ex: "a-z" into an equivalent form
        suitable for use in our regex engine.

        :param range_expr: character range expression to expand
        :type range_expr: str

        :return: the expanded form of the range expression
        :rtype: str
        """
        if not '-' in range_expr:
            raise Exception('range_expand: unsupported regex given')
        start, end = range_expr.split('-')
        if ord(end) < ord(start):
            raise Exception('range_expand: unsupported regex given')
        elements = [chr(i) for i in range(ord(start), ord(end)+1)]
        return f'({"|".join(elements)})' 

    @staticmethod
    def preprocess(regex: str) -> str:
        """Performs preprocessing of a regular expression into an expanded form
        suitable for use in our engine. This function expands common syntactical
        sugar used in regex libraries and produces an equivalent regex in fully
        expanded form.

        :param regex: a supported regular expression for our engine
        :type regex: str

        :return: the expanded form of the regular expression
        :rtype: str
        """
        # First pass, expand all (simple) character classes
        char_classes = re.findall(r'\[(.*?)\]', regex)
        for cc in char_classes:
            if re.match(r'^.\-.$', cc):
                regex = regex.replace(f'[{cc}]', DankEncoder.range_expand(cc))
            else:
                raise Exception('preprocess: unsupported regex given')
        # Second pass, expand all repetition operators. We have to update the
        # parsing index "i" to point to the correct location in the regex after
        # the mutation. "idx" is a supplemental counter which is only updated
        # after we have completely parse an entire expression, it is used to
        # keep us from modifying parts of the final regex more than once.
        i, idx = 0, 0
        parse_stack, expr_stack, rep_stack = [], [], []
        while i < len(regex):
            e = regex[i]
            if e == '(':
                parse_stack.append(i)
            elif e == ')':
                start = parse_stack.pop()
                expr = regex[start+1:i]
                expr_stack.append(expr)
            elif e == '{':
                rep_stack.append(i)
            elif e == '}':
                expr = expr_stack.pop()
                start = rep_stack.pop()
                repe = regex[start+1:i]
                if re.match(r'^(\d+)$', repe):
                    n = int(re.match(r'^(\d+)$', repe)[0])
                    e = f'({expr})'
                    j = regex.index(e, idx)
                    regex = regex[:j] + (e*n) + regex[j+len(e):]
                    regex = regex.replace('{%s}' % n, '', 1)
                    i = (i-(len(repe)+2)) + len(e*(n-1))
                    if len(parse_stack) == 0:
                        idx += len(e*n)
                elif re.match(r'^(\d+),$', repe):
                    n = int(re.match(r'^(\d+),$', repe)[0][:-1])
                    e = f'({expr})'
                    j = regex.index(e, idx)
                    regex = regex[:j] + (e*(n+1)+'*') + regex[j+len(e):]
                    regex = regex.replace('{%s,}' % n, '', 1)
                    i = (i-(len(repe)+2)) + len(e*n) + 1
                    if len(parse_stack) == 0:
                        idx += len(e*(n+1)+'*')
                elif re.match(r'^(\d+),(\d+)$', repe):
                    a, b = map(int, re.match(r'^(\d+),(\d+)$', repe)[0].split(','))
                    e = f'({expr})'
                    r = f'({"|".join(["(" + e*k + ")" for k in range(b,a-1,-1)])})'
                    j = regex.index(e, idx)
                    regex = regex[:j] + r + regex[j+len(e):]
                    regex = regex.replace('{%s,%s}' % (a, b), '', 1)
                    i = (i-len(repe)+2) + (sum(range(a,b+1))-1)*len(e) + (b-a) + (b-a+1) + 2
                    if len(parse_stack) == 0:
                        idx += len(r)
            i += 1
        return regex
