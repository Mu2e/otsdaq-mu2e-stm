WARNING="-Wall -Wformat=2 -Wno-format-nonliteral -Wno-strict-aliasing -Wuninitialized -Wno-unused-function -Wextra -Wformat=2 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-pointer-arith -Wno-unused-variable -Wno-unused-local-typedefs -Wno-parentheses -Wno-deprecated-declarations -Wno-conversion-null -Wno-unused-but-set-variable -Wno-type-limits -Wno-unused-but-set-parameter -Wno-unused-value -fPIC -Wno-deprecated -Wno-sign-compare"
CCOPTS="-O3"

echo "g++ -std=c++11 -pthread $CCOPTS $WARNING dataVars.cc queue.cc queue_zs.cc genData.cc zeroSuppress.cc testZSdaq.cc -o run.exe"
g++ -std=c++11 -pthread $CCOPTS $WARNING dataVars.cc queue.cc queue_zs.cc genData.cc zeroSuppress.cc testZSdaq.cc -o run.exe

