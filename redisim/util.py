import operator
from functools import reduce


def merge_args(*args, **kwargs):
    _kwargs = [(k, v) for k, v in kwargs.items() if v is not None]
    _kwargs = _kwargs if len(_kwargs) > 0 else [()]
    return operator.add(args, reduce(operator.add, _kwargs))

def chunks(l, n=2):
    """Yield successive n-sized chunks from l."""
    for i in range(0, len(l), n):
        yield l[i:i + n]
