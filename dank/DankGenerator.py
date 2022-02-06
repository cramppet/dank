import random
from typing import Optional

try:
    from .DankEncoder import DankEncoder
except ImportError as ex:
    print(f'dank module failed to load: {ex}')


class DankGenerator:
    def __init__(self, regex: str, fixed_slice: Optional[int] = None, number: int = 0, random: bool = False):
        t = DankEncoder(regex, 1)
        fs = t.num_states()-2
        self.fixed_slice = fixed_slice if fixed_slice else fs
        self.encoder = DankEncoder(regex, self.fixed_slice)
        self.words = self.encoder.num_words(fs, fs)
        self.number = number if number else self.words
        self.random = random
        self.count = 0

    def __iter__(self):
        return self

    def __next__(self):
        if self.count >= self.number:
            raise StopIteration
        self.count += 1
        if self.random:
            return self.encoder.unrank(random.randint(0, self.words))
        return self.encoder.unrank(self.count-1)
