
\ new statement
0
lang.System._staticBlocking
!

\ new statement
0
lang.System._staticThread
!

\ new statement
lang.System§866276096.table 1870680832 EXECUTE-NEW
lang.System._this
OVER 0 V! !

\ new statement
( *
* The "standard" input stream. This stream is already
* open and ready to supply input data. Typically this stream
* corresponds to keyboard input or another input source specified by
* the host environment or user.
)

\ new statement
std17817 VAL
lang.System.nullInputStream§-746820096
lang.System.in
OVER 0 V! ! FALSE DUP
IF
   std17817 LABEL TRUE
ENDIF
IF
   lang.JavaArray.handler§-1096259584
ENDIF

\ new statement
( *
* The "standard" output stream. This stream is already
* open and ready to accept output data. Typically this stream
* corresponds to display output or another output destination
* specified by the host environment or user.
* <p>
* For simple stand-alone Java applications, a typical way to write
* a line of output data is:
* <blockquote><pre>
*     System.out.println(data)
* </pre></blockquote>
* <p>
* See the <code>println</code> methods in class <code>PrintStream</code>.
*
* @see     java.io.PrintStream#println()
* @see     java.io.PrintStream#println(boolean)
* @see     java.io.PrintStream#println(char)
* @see     java.io.PrintStream#println(char[])
* @see     java.io.PrintStream#println(double)
* @see     java.io.PrintStream#println(float)
* @see     java.io.PrintStream#println(int)
* @see     java.io.PrintStream#println(long)
* @see     java.io.PrintStream#println(java.lang.Object)
* @see     java.io.PrintStream#println(java.lang.String)
)

\ new statement
std17819 VAL
lang.System.nullPrintStream§-182292992
lang.System.out
OVER 0 V! ! FALSE DUP
IF
   std17819 LABEL TRUE
ENDIF
IF
   lang.JavaArray.handler§-1096259584
ENDIF

\ new statement
( *
* The "standard" error output stream. This stream is already
* open and ready to accept output data.
* <p>
* Typically this stream corresponds to display output or another
* output destination specified by the host environment or user. By
* convention, this output stream is used to display error messages
* or other information that should come to the immediate attention
* of a user even if the principal output stream, the value of the
* variable <code>out</code>, has been redirected to a file or other
* destination that is typically not continuously monitored.
)

\ new statement
std17821 VAL
lang.System.nullPrintStream§-182292992
lang.System.err
OVER 0 V! ! FALSE DUP
IF
   std17821 LABEL TRUE
ENDIF
IF
   lang.JavaArray.handler§-1096259584
ENDIF
