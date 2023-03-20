import re
import sys

if len(sys.argv) > 1:
    with open(sys.argv[1], mode='r') as f:
        all_file = f.read()
else:
    all_file = sys.stdin.read()

indent = ""
no_end_line = -1
no_write = -1
for line in re.split('([,<>])', all_file):
    line = line.strip()
    has_std = 'std::' in line and 'std::tuple' not in line
    can_skip = 'std::less' in line or 'std::hash' in line or 'std::equal' in line or 'allocator' in line or \
               'char_traits' in line or 'integer_sequence' in line
    line = line.split(':')[-1]
    if not line:
        continue

    if line == '<':
        if no_write != -1:
            no_write += 1
            continue

        indent += "  "
        sys.stdout.write('<')
    elif line == ',':
        if no_write != -1:
            continue

        sys.stdout.write(', ')
    elif line == '>':
        if no_write != -1:
            no_write -= 1
            if not no_write:
                no_write = -1
            continue

        indent = indent[:-2]
        if no_end_line == -1:
            sys.stdout.write('\n' + indent)

        sys.stdout.write('>')

        if len(indent) == no_end_line:
            no_end_line = -1
    else:
        if no_write != -1:
            continue

        if can_skip:
            sys.stdout.write('T')
            no_write = 0
            continue

        if no_end_line == -1 and indent:
            sys.stdout.write('\n' + indent)
        sys.stdout.write(line)

        if no_end_line == -1 and (has_std or 'tuple_like' in line or 'map_save' in line):
            no_end_line = len(indent)

sys.stdout.write('\n')
sys.stdout.flush()
