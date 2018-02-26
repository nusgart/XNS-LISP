# XNS-LISP (Xenon Simple Lisp)
XNS-LISP is a basic lisp implementation for learning purposes in implementing
Lisp, not learning Lisp itself.  If you want to learn Lisp, then you should
learn a standard dialect like Common Lisp, Scheme, or Clojure (EMACS Lisp has
an older design that uses default dynamic scoping and doesn't have closures,
but IS very useful because of EMACS). 

XNS-Lisp is a Lisp 1 that will otherwise be somewhat similar to Common Lisp. It
uses CheneyGC for its garbage collector, which means that it is a precise GC --
so if you want to interface with it using C, see the handle API (or the root
pointer registration API, which is more complicated and harder to use).
## License
Copyright 2018- Nicholas Nusgart, All Rights Reserved

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

### Explanation
  In the tradition of Lisp implementations such as CMUCL and SBCL, I would like
  to put as few restrictions as possible on XNS-LISP.  Hence, I license XNS-LISP
  under the Apache 2.0 License.  
## TODO: (Neccesary things/tools/...)
* List structured memory -- Mostly done 
* Garbage collection  -- CheneyGC implemented
* Environment handling -- initial 
* S-Expression I/O -- both input & output (still fragile)
* Source file loading -- the reader can read from any FILE* (but no eval)
* EVAL/APPLY/EVALIS -- TODO
* Primitives -- TODO
* REPL -- TODO (need EVAL, Primitives, and S-Expression I/O first) (R,P,L work)
* Standard Library -- TODO (need EVAL/APPLY/EVALIS, Primitives, and Source file loading first)
## Authors:
* Nicholas Nusgart <nicknusgart@gmail.com>

## Credits
* Professor John McCarthy (Invented Lisp in 1958 among many other accomplishments)
* Rui Ueyama (github rui314) https://github.com/rui314/minilisp -- inspiration
* Wikipedia https://en.wikipedia.org/wiki/Cheney%27s_algorithm
* C.J. Cheney https://doi.org/10.1145%2F362790.362798 (Cheney's GC algorithm)
* Andru Luvisi http://web.sonoma.edu/users/l/luvisi/sl5.c -- inspiration