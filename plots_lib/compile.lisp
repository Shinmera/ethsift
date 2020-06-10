#|
sbcl --noinform --load "$0" --eval "(main)" --quit
exit
|#

#-quicklisp (load "~/quicklisp/setup.lisp")
(ql:quickload '(cl-ppcre) :silent T)

(defvar *here* #.(or *compile-file-pathname* *load-pathname*
                     (error "Please COMPILE-FILE or LOAD this file.")))

(defun log-dir ()
  (merge-pathnames "../logs_i7_8700K/" *here*))

(defun count-dir ()
  (merge-pathnames "../flops_logs/" *here*))

(defun parse-csv (path &optional (transform #'identity))
  (with-open-file (stream path :direction :input)
    (read-line stream)
    (loop for line = (read-line stream NIL)
          while line
          collect (apply transform (cl-ppcre:split ", *" line)))))

(defun extend-cols (extension rows)
  (loop for row in rows
        collect (append extension row)))

(defun filter-cols (row cols)
  (loop for col in cols
        collect (getf row col)))

(defun list-to-row (row cols)
  (loop for val in row
        for label in cols
        collect label collect val))

(defun find-matching (entry candidates &rest fields)
  (loop for candidate in candidates
        do (when (loop for field in fields
                       always (equalp (getf candidate field) (getf entry field)))
             (return candidate))))

(defun find-all-matching (entry candidates &rest fields)
  (loop for candidate in candidates
        when (loop for field in fields
                   always (equalp (getf candidate field) (getf entry field)))
        collect candidate))

(defun parse-filename-info (filename)
  (destructuring-bind (stamp mode input commit) (cl-ppcre:split "_" filename)
    (list :stamp stamp
          :mode mode
          :input input
          :commit commit)))

(defun parse-log-csv (path)
  (parse-csv path (lambda (function median mad)
                    (list :function (cl-ppcre:regex-replace "^.*?_" function "")
                          :median (read-from-string median)
                          :mad (read-from-string mad)))))

(defun parse-count-csv (path)
  (parse-csv path (lambda (function flops bytes)
                    (list :function function
                          :flops (read-from-string flops)
                          :bytes (read-from-string bytes)))))

(defun parse-log (path)
  (let* ((dirs (pathname-directory path))
         (compiler (nth (- (length dirs) 2) dirs)))
    (destructuring-bind (library version &optional flags) (cl-ppcre:split " " (nth (1- (length dirs)) dirs))
      (unless flags
        (shiftf flags version "baseline"))
      (setf flags (cl-ppcre:regex-replace "-flags$" flags ""))
      (extend-cols
       (append (parse-filename-info (pathname-name path))
               (list :library library
                     :version version
                     :flags flags
                     :compiler compiler))
       (parse-log-csv path)))))

(defun parse-count (path)
  (extend-cols
   (append (parse-filename-info (pathname-name path))
           (list :library "ethsift"
                 :version (car (last (pathname-directory path)))))
   (parse-count-csv path)))

(defun parse-all-logs (base)
  (loop for file in (directory (make-pathname :name :wild :type "csv" :defaults (merge-pathnames uiop:*wild-inferiors* base)))
        nconc (parse-log file)))

(defun parse-all-counts (base)
  (loop for file in (directory (make-pathname :name :wild :type "csv" :defaults (merge-pathnames uiop:*wild-inferiors* base)))
        nconc (parse-count file)))

(defun compute-performance (cycles mad flops)
  (let ((perf (float (/ flops cycles)))) ;; Copied from read_logs.py, but this seems weird af to me.
    (values perf (- (/ flops (+ mad cycles)) perf))))

(defun normalise-log (log logs cycle-counts)
  (when (string= "rdtsc" (getf log :mode))
    (let ((count (getf (find-matching log cycle-counts :version :function :input) :flops 0)))
      (multiple-value-bind (perf mad) (compute-performance (getf log :median) (getf log :mad) count)
        (setf (getf log :median) perf)
        (setf (getf log :mad) mad))))
  (let* ((others (find-all-matching log logs :library :version :compiler :flags :mode :input))
         (sum (loop for log in others
                    when (find (getf log :function) '("GaussianPyramid" "DOGPyramid" "GradientAndrotationpyramids" "Histogram" "ExtremaRefinement" "KeypointDetection" "ExtractDescriptor") :test #'equalp)
                    sum (getf log :median))))
    (setf (getf log :relative) (/ (getf log :median) sum)))
  (setf (getf log :input-size) (remove-if-not #'digit-char-p (getf log :input)))
  log)

(defun log< (a b)
  (flet ((pos (thing)
           (position thing '("auto-240p" "auto-360p" "auto-480p" "auto-720p" "auto-1080p" "auto-2160p" "auto-4320p")
                     :test #'string-equal)))
    (< (pos (getf a :input)) (pos (getf b :input)))))

(defun parse (logs counts)
  (let ((logs (parse-all-logs logs))
        (counts (parse-all-counts counts)))
    (sort (loop for log in logs collect (normalise-log log logs counts)) #'log<)))

(defun write-all-logs (logs path)
  (with-open-file (stream path :direction :output
                               :if-exists :supersede)
    (let ((cols '(:library :version :compiler :flags :mode :input :input-size :function :median :mad :relative)))
      (labels ((write-row (cols)
                 (format stream "~{~f~^, ~}~%" cols)))
        (write-row (list* :name cols))
        (dolist (row logs)
          (write-row (list* (format NIL "~a ~a ~a ~a ~a ~a"
                                    (getf row :library) (getf row :version)
                                    (getf row :compiler) (getf row :flags)
                                    (getf row :mode) (getf row :function))
                            (filter-cols row cols))))))))

(defun main ()
  (write-all-logs (parse (log-dir) (count-dir))
                  (merge-pathnames "all.csv" *here*)))
