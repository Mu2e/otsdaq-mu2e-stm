WARNING="-Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -W\
format=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-lo\
cal-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-typ\
e-limits -Wno-unused-but-set-parameter -Wno-unused-value -fPIC -Wno-deprecated -Wno-sign-compare"
CCOPTS="-O3"

g++ $CCOPTS $WARNING -o run.exe main.cc baseline.cc
