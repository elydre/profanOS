import os

stdout = os.fdopen(1, 'w')

print('Hello from test.py!', file=stdout)
print('This is a normal print')
