
\ new statement
0
equation.String._staticBlocking
!

\ new statement
( * use serialVersionUID from JDK 1.0.2 for interoperability  )

\ new statement
-6849794470754667710
equation.String.serialVersionUID
2!

\ new statement
( *
* A Comparator that orders <code>String</code> objects as by
* <code>compareToIgnoreCase</code>. This comparator is serializable.
* <p>
* Note that this Comparator does <em>not</em> take locale into account,
* and will result in an unsatisfactory ordering for certain locales.
* The java.text package provides <em>Collators</em> to allow
* locale-sensitive ordering.
*
* @see     java.text.Collator#compare(String, String)
* @since   1.2
)

\ new statement
equation.String.CaseInsensitiveComparator§-2000076032.table -42253568 EXECUTE-NEW
equation.String.CASE_INSENSITIVE_ORDER
DUP @ TRUE equation.JavaArray.kill§1620077312 !

\ new statement
0
equation.String.CaseInsensitiveComparator._staticBlocking
!

\ new statement
\  use serialVersionUID from JDK 1.2.2 for interoperability


\ new statement
8575799808933029326
equation.String.CaseInsensitiveComparator.serialVersionUID
2!
