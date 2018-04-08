;;;;;;; XNS Lisp Standard Definitions: Essential definitions -- these are loaded first.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright 2018- Nicholas Nusgart, All Rights Reserved
;  Licensed under the Apache License, Version 2.0 (the "License");
;  you may not use this file except in compliance with the License.
;  You may obtain a copy of the License at
;      http://www.apache.org/licenses/LICENSE-2.0
;  Unless required by applicable law or agreed to in writing, software
;  distributed under the License is distributed on an "AS IS" BASIS,
;  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;  See the License for the specific language governing permissions and
;  limitations under the License.

(define list (lambda (&rest l) l))

(define defmacro (mlambda (fname args &rest code)
    (list 'define fname (append (list 'mlambda args) code) )))

(defmacro funcall (fname &rest args) (cons fname args))

(defmacro defun (fname args &rest code)
    (list 'define fname (append (list 'lambda args) code)))

(defmacro if (condition p1 &rest p2)
    (list 'cond (list condition p1) (list (list T) p2)))

(defun fold (f i xs) 
    (if (null xs) 
        i
        (fold f (f i (car xs) ) (cdr xs))))

(defun reduce (f xs) (fold f NIL xs))