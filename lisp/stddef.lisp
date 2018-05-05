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
;;conventional names for setcar and setcdr
(define rplaca setcar)
(define rplacd setcdr)
;; defmacro is the operator which defines macros. (mlambda is an implementation detail)
(define defmacro (mlambda (fname args &rest code)
			  (list 'define fname (append (list 'mlambda
			  args) code) )))

;; funcall is like apply but takes its arguements inline
;; since XNS is a Lisp-1, this isn't actually necessary
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

;; if -- if condition execute p1 else execute p2.
(defmacro if (condition p1 &rest p2)
  (list 'cond (list condition p1) (cons T p2)))
;; when --> if condition, execute (progn arguments)
(defmacro when (condition &rest p1)
  (list 'cond (list condition (cons 'progn p1))))
;; unless --> if not condition, execute (progn arguments)
(defmacro unless (condition &rest p1)
  (list 'cond (list (list 'not condition) (cons 'progn p1))))

;; last --> returns the last item in a list
(defun last (list)
  (if (cdr list) (last (cdr list)) list))
;; nconc --> destructive version of append
(defun nconc (&rest lists)
  (cond ((null lists)'())
    ((null (cdr lists)) lists)
    ((null (cddr lists)) (progn (rplacd (last (car lists)) (cadr lists)) (car lists)))
    (t (nconc (nconc (car lists) (cadr lists)) lists))
 )))

;; fold
(defun fold (f i xs) 
  (if (null xs) 
      i
      (fold f (f i (car xs) ) (cdr xs))))
;; reduce
(defun reduce (f xs) (fold f NIL xs))
;; map: currently does nothing
(defun map (result-type function &rest sequences)
  'nil)
;; mapcar --> maps a function over lists and appends the results together
(defun mapcar (function lst)
  (if (null lst) nil
    (append (function (car lst)) (mapcar function (cdr lst)))))
;; mapcan: like mapcar but uses nconc, so is destructive!
(defun mapcan (function lst)
  (if (null lst) nil
    (nconc (function (car lst)) (mapcan function (cdr lst)))))
;; for?
(defmacro for (varlist &rest code)
  (list (list 'define (car varlist) (cadr varlist))
        (list 'while (list '< (caddr varlist)) code) ))
;; This should act as a for loop?      
(defun for (varlist &rest code)
  (list (list 'define (car varlist) (cadr varlist))
        (list 'while
          (cons (list '< (car varlist) (caddr varlist)) 
            (append code (list (list 'define (car varlist) (list '+ (car varlist) 1))) ) ) ) ))

; setf macro: use this to set values
(defmacro setf (code value)
  (if (atom code) (list 'set code value)
    (cons (intern (+ "setf$" (car code))) (append (cdr code) (list value)) ) ))

; push
(defmacro push (val lst)
  (list 'set lst (list 'cons val lst)))

; setf "functions"
(define setf$car setcar)
(define setf$cdr setcdr)
(defmacro setf$assoc (key lst value)
  (list 'push (cons key value) lst))

;;;; hash map???