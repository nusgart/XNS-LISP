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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; list creates a list of the arguements
(define list (lambda (&rest l) l))

;; defmacro is the operator which defines macros. 
(define defmacro (mlambda (fname args &rest code)
			  (list 'define fname (append (list 'mlambda
			  args) code) )))

;; funcall is like apply but takes its arguements inline
(defmacro funcall (fname &rest args) (cons fname args))

;; defun defines named functions
(defmacro defun (fname args &rest code)
  (list 'define fname (append (list 'lambda args) code)))

;; some really essential definitions
(defun caar (x) (car (car x)))
(defun cadr (x) (car (cdr x)))
(defun cdar (x) (cdr (car x)))
(defun cddr (x) (cdr (cdr x)))

(defun caaar (x) (car (car (car x))))
(defun caadr (x) (car (car (cdr x))))
(defun cadar (x) (car (cdr (car x))))
(defun caddr (x) (car (cdr (cdr x))))

(defun cdaar (x) (cdr (car (car x))))
(defun cdadr (x) (cdr (car (cdr x))))
(defun cddar (x) (cdr (cdr (car x))))
(defun cdddr (x) (cdr (cdr (cdr x))))
;; if -- 
(defmacro if (condition p1 &rest p2)
  (list 'cond (list condition p1) (list (list T) p2)))

;; fold
(defun fold (f i xs) 
  (if (null xs) 
      i
      (fold f (f i (car xs) ) (cdr xs))))

(defun reduce (f xs) (fold f NIL xs))

(defun map (result-type function &rest sequences)
  'nil)

(defun mapcan (function &rest lists)
  (cons (function (car list)) (mapcan function (cdr list))))

(defmacro for (varlist &rest code)
  (list (list 'define (car varlist) (cadr varlist))
        (list 'while (list '< (caddr varlist)) code) ))
        
(defun for (varlist &rest code)
  (list (list 'define (car varlist) (cadr varlist))
        (list 'while
          (cons (list '< (car varlist) (caddr varlist)) 
            (append code (list (list 'define (car varlist) (list '+ (car varlist) 1))) ) ) ) ))
