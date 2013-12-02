------------------------------------------------------------------------
			 TIPA -- TIny PArser
------------------------------------------------------------------------

VERSION: 0.1 (alpha)

Author: Giuseppe Lipari (giulipari@gmail.com)

* INTRODUCTION

  This is a library for implementing simple parsers in C++.  Since I
  often build simulators, tools for analysis, and recently also a
  small model checker, I often need to read a configuration file
  various formats. Therefore, after looking around, I decided to try
  to use the Boost::Spirit parser.

  I have to say that I have mixed feelings about Spirit: I love the
  concise syntax of the rules, the flexibility, and the many features
  and tools available. However, I hate the steep learning curve, and I
  hate the fact that, every time I try to use it, I lose so much time
  in debugging.

  Therefore, I decided to write a very simple library as replacement
  for Spirit. I tried to keep the same "spirit" and use a simple
  technique to write the rules in a simil-BNF style. However, the
  enphasis is in simplicity of use, therefore I kept the set of
  features to a minimal level, and most importantly, I did not try to
  optimize the parsing process.

  Therefore, if you have to write a simple parser for a short file,
  and create a simple AST, this may be the library you need, give it a
  try!

  It requires the Boost::regex library, and uses the C++11 syntax, so
  make sure to use an upo-to-date compiler.

  - To compile and install, just go trough the usual 
    
    ./configure && make && make install

  - You can run the tests with in ./test/test

  - Two examples are available in example/arithmetic and
    example/tinyjson

  Have fun!

* PROGRAMMER MANUAL 

  To be done...


