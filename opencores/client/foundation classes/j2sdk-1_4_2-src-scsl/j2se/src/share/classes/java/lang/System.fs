MODULENAME java.lang.System
(
* @(#)System.java	1.131 03/01/29
*
* Copyright 2003 Sun Microsystems, Inc. All rights reserved.
* SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
)  \ import java.util.Properties;
\ import java.util.PropertyPermission;
\ import java.util.StringTokenizer;
\ import java.security.AccessController;
\ import java.security.PrivilegedAction;
\ import java.security.AllPermission;
\ import sun.net.InetAddressCachePolicy;
\ import sun.reflect.Reflection;
\ import sun.security.util.SecurityConstants;
( *
* The <code>System</code> class contains several useful class fields
* and methods. It cannot be instantiated.
* <p>
* Among the facilities provided by the <code>System</code> class
* are standard input, standard output, and error output streams;
* access to externally defined "properties"; a means of
* loading files and libraries; and a utility method for quickly
* copying a portion of an array.
*
* @author  Arthur van Hoff
* @version 1.131, 01/29/03
* @since   JDK1.0
)
( * Don't let anyone instantiate this class  )

:LOCAL lang.System.System§1870680832
   1 VALLOCATE LOCAL §base0
   
   \ new statement
   0
   LOCALS §this |
   
   \ new statement
   lang.Object§-1890496768.table 1827294976 EXECUTE-NEW
   
   \ new statement
   28 MALLOC DROP DUP DUP §base0 V! TO §this OVER
   IF
      OVER CELL+ @ 4 + OVER OVER ! DROP
   ENDIF
   CELL+ @ OVER
   OVER ! NIP
   lang.System§866276096.table OVER 12 + !
   866254848 OVER 20 + !
   " System " OVER 16 + !
   0 OVER 24 + ! DROP
   
   \ new statement
   lang.System.initializeSystemClass§-2071172864
   
   \ new statement
   §this DUP 0 V! ( return object )    
   \ new statement
   0 §break17244 LABEL
   
   \ new statement
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Copies an array from the specified source array, beginning at the
* specified position, to the specified position of the destination array.
* A subsequence of array components are copied from the source
* array referenced by <code>src</code> to the destination array
* referenced by <code>dest</code>. The number of components copied is
* equal to the <code>length</code> argument. The components at
* positions <code>srcPos</code> through
* <code>srcPos+length-1</code> in the source array are copied into
* positions <code>destPos</code> through
* <code>destPos+length-1</code>, respectively, of the destination
* array.
* <p>
* If the <code>src</code> and <code>dest</code> arguments refer to the
* same array object, then the copying is performed as if the
* components at positions <code>srcPos</code> through
* <code>srcPos+length-1</code> were first copied to a temporary
* array with <code>length</code> components and then the contents of
* the temporary array were copied into positions
* <code>destPos</code> through <code>destPos+length-1</code> of the
* destination array.
* <p>
* If <code>dest</code> is <code>null</code>, then a
* <code>NullPointerException</code> is thrown.
* <p>
* If <code>src</code> is <code>null</code>, then a
* <code>NullPointerException</code> is thrown and the destination
* array is not modified.
* <p>
* Otherwise, if any of the following is true, an
* <code>ArrayStoreException</code> is thrown and the destination is
* not modified:
* <ul>
* <li>The <code>src</code> argument refers to an object that is not an
*     array.
* <li>The <code>dest</code> argument refers to an object that is not an
*     array.
* <li>The <code>src</code> argument and <code>dest</code> argument refer
*     to arrays whose component types are different primitive types.
* <li>The <code>src</code> argument refers to an array with a primitive
*    component type and the <code>dest</code> argument refers to an array
*     with a reference component type.
* <li>The <code>src</code> argument refers to an array with a reference
*    component type and the <code>dest</code> argument refers to an array
*     with a primitive component type.
* </ul>
* <p>
* Otherwise, if any of the following is true, an
* <code>IndexOutOfBoundsException</code> is
* thrown and the destination is not modified:
* <ul>
* <li>The <code>srcPos</code> argument is negative.
* <li>The <code>destPos</code> argument is negative.
* <li>The <code>length</code> argument is negative.
* <li><code>srcPos+length</code> is greater than
*     <code>src.length</code>, the length of the source array.
* <li><code>destPos+length</code> is greater than
*     <code>dest.length</code>, the length of the destination array.
* </ul>
* <p>
* Otherwise, if any actual component of the source array from
* position <code>srcPos</code> through
* <code>srcPos+length-1</code> cannot be converted to the component
* type of the destination array by assignment conversion, an
* <code>ArrayStoreException</code> is thrown. In this case, let
* <b><i>k</i></b> be the smallest nonnegative integer less than
* length such that <code>src[srcPos+</code><i>k</i><code>]</code>
* cannot be converted to the component type of the destination
* array; when the exception is thrown, source array components from
* positions <code>srcPos</code> through
* <code>srcPos+</code><i>k</i><code>-1</code>
* will already have been copied to destination array positions
* <code>destPos</code> through
* <code>destPos+</code><i>k</I><code>-1</code> and no other
* positions of the destination array will have been modified.
* (Because of the restrictions already itemized, this
* paragraph effectively applies only to the situation where both
* arrays have component types that are reference types.)
*
* @param      src      the source array.
* @param      srcPos   starting position in the source array.
* @param      dest     the destination array.
* @param      destPos  starting position in the destination data.
* @param      length   the number of array elements to be copied.
* @exception  IndexOutOfBoundsException  if copying would cause
*               access of data outside array bounds.
* @exception  ArrayStoreException  if an element in the <code>src</code>
*               array could not be stored into the <code>dest</code> array
*               because of a type mismatch.
* @exception  NullPointerException if either <code>src</code> or
*               <code>dest</code> is <code>null</code>.
)

: lang.System.arraycopy§1260740864
   4 VALLOCATE LOCAL §base0
   LOCAL length
   LOCAL destPos
   DUP 0 §base0 + V! LOCAL dest
   LOCAL srcPos
   DUP 4 §base0 + V! LOCAL src
   
   \ new statement
   0 DUP DUP DUP
   LOCALS d dstart s sstart |
   
   \ new statement
   src
   DUP 8 §base0 + V! TO s
   
   \ new statement
   dest
   DUP 12 §base0 + V! TO d
   
   \ new statement
   s A:R@
   srcPos
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO sstart
   
   \ new statement
   d A:R@
   destPos
   R> -942971136 TRUE ( java.lang.JavaArray.getElem§-942971136 ) EXECUTE-METHOD
   TO dstart
   
   \ new statement
   length  S>D
   d CELL+ @ 32 + ( java.lang.JavaArray.shift )    @
   SHIFTL  D>S
   TO length
   
   \ new statement
   sstart dstart length MOVE
   
   \ new statement
   0 §break17253 LABEL
   
   \ new statement
   
   
   
   
   
   
   
   
   
   §base0 SETVTOP
   PURGE 10
   
   \ new statement
   DROP
;


: lang.System.checkIO§1395811072
   
   \ new statement
   
   \ new statement
   \    if (security != null)
   \ 	    security.checkPermission(new RuntimePermission("setIO"));
   
   
   \ new statement
   0 §break17248 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.System.classMonitorEnter§-433167616
   
   \ new statement
   
   \ new statement
   0 §break17242 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;


: lang.System.classMonitorLeave§-1691393280
   
   \ new statement
   
   \ new statement
   0 §break17243 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Sets the System security.
*
* <p> If there is a security manager already installed, this method first
* calls the security manager's <code>checkPermission</code> method
* with a <code>RuntimePermission("setSecurityManager")</code>
* permission to ensure it's ok to replace the existing
* security manager.
* This may result in throwing a <code>SecurityException</code>.
*
* <p> Otherwise, the argument is established as the current
* security manager. If the argument is <code>null</code> and no
* security manager has been established, then no action is taken and
* the method simply returns.
*
* @param      s   the security manager.
* @exception  SecurityException  if the security manager has already
*             been set and its <code>checkPermission</code> method
*             doesn't allow it to be replaced.
* @see #getSecurityManager
* @see SecurityManager#checkPermission
* @see java.lang.RuntimePermission
public static
void setSecurityManager(final SecurityManager s) {
try {
s.checkPackageAccess("java.lang");
} catch (Exception e) {
// no-op
}
setSecurityManager0(s);
}

private static synchronized
void setSecurityManager0(final SecurityManager s) {
if (security != null) {
// ask the currently installed security manager if we
// can replace it.
security.checkPermission(new RuntimePermission
("setSecurityManager"));
}

if ((s != null) && (s.getClass().getClassLoader() != null)) {
// New security manager class is not on bootstrap classpath.
// Cause policy to get initialized before we install the new
// security manager, in order to prevent infinite loops when
// trying to initialize the policy (which usually involves
// accessing some security and/or system properties, which in turn
// calls the installed security manager's checkPermission method
// which will loop infinitely if there is a non-system class
// (in this case: the new security manager class) on the stack).
AccessController.doPrivileged(new PrivilegedAction() {
public Object run() {
s.getClass().getProtectionDomain().implies
(SecurityConstants.ALL_PERMISSION);
return null;
}
});
}

security = s;
InetAddressCachePolicy.setIfNotSet(InetAddressCachePolicy.FOREVER);
}
)  ( *
* Gets the system security interface.
*
* @return  if a security manager has already been established for the
*          current application, then that security manager is returned;
*          otherwise, <code>null</code> is returned.
* @see     #setSecurityManager
public static SecurityManager getSecurityManager() {
return security;
}
)  ( *
* Returns the current time in milliseconds.  Note that
* while the unit of time of the return value is a millisecond,
* the granularity of the value depends on the underlying
* operating system and may be larger.  For example, many
* operating systems measure time in units of tens of
* milliseconds.
*
* <p> See the description of the class <code>Date</code> for
* a discussion of slight discrepancies that may arise between
* "computer time" and coordinated universal time (UTC).
*
* @return  the difference, measured in milliseconds, between
*          the current time and midnight, January 1, 1970 UTC.
* @see     java.util.Date
)

: lang.System.currentTimeMillis§1830576896
   
   \ new statement
   
   \ new statement
   MILLISECONDS 2@
   
   \ new statement
   0 §break17252 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Terminates the currently running Java Virtual Machine. The
* argument serves as a status code; by convention, a nonzero status
* code indicates abnormal termination.
* <p>
* This method calls the <code>exit</code> method in class
* <code>Runtime</code>. This method never returns normally.
* <p>
* The call <code>System.exit(n)</code> is effectively equivalent to
* the call:
* <blockquote><pre>
* Runtime.getRuntime().exit(n)
* </pre></blockquote>
*
* @param      status   exit status.
* @throws  SecurityException
*        if a security manager exists and its <code>checkExit</code>
*        method doesn't allow exit with the specified status.
* @see        java.lang.Runtime#exit(int)
)

: lang.System.exit§879453440
   LOCAL status
   
   \ new statement
   
   \ new statement
   \ Runtime.getRuntime().exit(status);
   
   
   \ new statement
   ABORT" program aborted "
   
   \ new statement
   0 §break17256 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Runs the garbage collector.
* <p>
* Calling the <code>gc</code> method suggests that the Java Virtual
* Machine expend effort toward recycling unused objects in order to
* make the memory they currently occupy available for quick reuse.
* When control returns from the method call, the Java Virtual
* Machine has made a best effort to reclaim space from all discarded
* objects.
* <p>
* The call <code>System.gc()</code> is effectively equivalent to the
* call:
* <blockquote><pre>
* Runtime.getRuntime().gc()
* </pre></blockquote>
*
* @see     java.lang.Runtime#gc()
)

: lang.System.gc§-779065600
   
   \ new statement
   
   \ new statement
   \ Runtime.getRuntime().gc();
   
   
   \ new statement
   0 §break17257 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* System properties. The following properties are guaranteed to be defined:
* <dl>
* <dt>java.version		<dd>Java version number
* <dt>java.vendor		<dd>Java vendor specific string
* <dt>java.vendor.url	<dd>Java vendor URL
* <dt>java.home		<dd>Java installation directory
* <dt>java.class.version	<dd>Java class version number
* <dt>java.class.path	<dd>Java classpath
* <dt>os.name		<dd>Operating System Name
* <dt>os.arch		<dd>Operating System Architecture
* <dt>os.version		<dd>Operating System Version
* <dt>file.separator	<dd>File separator ("/" on Unix)
* <dt>path.separator	<dd>Path separator (":" on Unix)
* <dt>line.separator	<dd>Line separator ("\n" on Unix)
* <dt>user.name		<dd>User account name
* <dt>user.home		<dd>User home directory
* <dt>user.dir		<dd>User's current working directory
* </dl>
)  \ private static Properties props;
\ private static native Properties initProperties(Properties props);
( *
* Determines the current system properties.
* <p>
* First, if there is a security manager, its
* <code>checkPropertiesAccess</code> method is called with no
* arguments. This may result in a security exception.
* <p>
* The current set of system properties for use by the
* {@link #getProperty(String)} method is returned as a
* <code>Properties</code> object. If there is no current set of
* system properties, a set of system properties is first created and
* initialized. This set of system properties always includes values
* for the following keys:
* <table summary="Shows property keys and associated values">
* <tr><th>Key</th>
*     <th>Description of Associated Value</th></tr>
* <tr><td><code>java.version</code></td>
*     <td>Java Runtime Environment version</td></tr>
* <tr><td><code>java.vendor</code></td>
*     <td>Java Runtime Environment vendor</td></tr
* <tr><td><code>java.vendor.url</code></td>
*     <td>Java vendor URL</td></tr>
* <tr><td><code>java.home</code></td>
*     <td>Java installation directory</td></tr>
* <tr><td><code>java.vm.specification.version</code></td>
*     <td>Java Virtual Machine specification version</td></tr>
* <tr><td><code>java.vm.specification.vendor</code></td>
*     <td>Java Virtual Machine specification vendor</td></tr>
* <tr><td><code>java.vm.specification.name</code></td>
*     <td>Java Virtual Machine specification name</td></tr>
* <tr><td><code>java.vm.version</code></td>
*     <td>Java Virtual Machine implementation version</td></tr>
* <tr><td><code>java.vm.vendor</code></td>
*     <td>Java Virtual Machine implementation vendor</td></tr>
* <tr><td><code>java.vm.name</code></td>
*     <td>Java Virtual Machine implementation name</td></tr>
* <tr><td><code>java.specification.version</code></td>
*     <td>Java Runtime Environment specification  version</td></tr>
* <tr><td><code>java.specification.vendor</code></td>
*     <td>Java Runtime Environment specification  vendor</td></tr>
* <tr><td><code>java.specification.name</code></td>
*     <td>Java Runtime Environment specification  name</td></tr>
* <tr><td><code>java.class.version</code></td>
*     <td>Java class format version number</td></tr>
* <tr><td><code>java.class.path</code></td>
*     <td>Java class path</td></tr>
* <tr><td><code>java.library.path</code></td>
*     <td>List of paths to search when loading libraries</td></tr>
* <tr><td><code>java.io.tmpdir</code></td>
*     <td>Default temp file path</td></tr>
* <tr><td><code>java.compiler</code></td>
*     <td>Name of JIT compiler to use</td></tr>
* <tr><td><code>java.ext.dirs</code></td>
*     <td>Path of extension directory or directories</td></tr>
* <tr><td><code>os.name</code></td>
*     <td>Operating system name</td></tr>
* <tr><td><code>os.arch</code></td>
*     <td>Operating system architecture</td></tr>
* <tr><td><code>os.version</code></td>
*     <td>Operating system version</td></tr>
* <tr><td><code>file.separator</code></td>
*     <td>File separator ("/" on UNIX)</td></tr>
* <tr><td><code>path.separator</code></td>
*     <td>Path separator (":" on UNIX)</td></tr>
* <tr><td><code>line.separator</code></td>
*     <td>Line separator ("\n" on UNIX)</td></tr>
* <tr><td><code>user.name</code></td>
*     <td>User's account name</td></tr>
* <tr><td><code>user.home</code></td>
*     <td>User's home directory</td></tr>
* <tr><td><code>user.dir</code></td>
*     <td>User's current working directory</td></tr>
* </table>
* <p>
* Multiple paths in a system property value are separated by the path
* separator character of the platform.
* <p>
* Note that even if the security manager does not permit the
* <code>getProperties</code> operation, it may choose to permit the
* {@link #getProperty(String)} operation.
*
* @return     the system properties
* @exception  SecurityException  if a security manager exists and its
*             <code>checkPropertiesAccess</code> method doesn't allow access
*              to the system properties.
* @see        #setProperties
* @see        java.lang.SecurityException
* @see        java.lang.SecurityManager#checkPropertiesAccess()
* @see        java.util.Properties
public static Properties getProperties() {
if (security != null) {
security.checkPropertiesAccess();
}
return props;
}
)  ( *
* Sets the system properties to the <code>Properties</code>
* argument.
* <p>
* First, if there is a security manager, its
* <code>checkPropertiesAccess</code> method is called with no
* arguments. This may result in a security exception.
* <p>
* The argument becomes the current set of system properties for use
* by the {@link #getProperty(String)} method. If the argument is
* <code>null</code>, then the current set of system properties is
* forgotten.
*
* @param      props   the new system properties.
* @exception  SecurityException  if a security manager exists and its
*             <code>checkPropertiesAccess</code> method doesn't allow access
*              to the system properties.
* @see        #getProperties
* @see        java.util.Properties
* @see        java.lang.SecurityException
* @see        java.lang.SecurityManager#checkPropertiesAccess()
public static void setProperties(Properties props) {
if (security != null) {
security.checkPropertiesAccess();
}
if (props == null) {
props = new Properties();
initProperties(props);
}
System.props = props;
}
)  ( *
* Gets the system property indicated by the specified key.
* <p>
* First, if there is a security manager, its
* <code>checkPropertyAccess</code> method is called with the key as
* its argument. This may result in a SecurityException.
* <p>
* If there is no current set of system properties, a set of system
* properties is first created and initialized in the same manner as
* for the <code>getProperties</code> method.
*
* @param      key   the name of the system property.
* @return     the string value of the system property,
*             or <code>null</code> if there is no property with that key.
*
* @exception  SecurityException  if a security manager exists and its
*             <code>checkPropertyAccess</code> method doesn't allow
*              access to the specified system property.
* @exception  NullPointerException if <code>key</code> is
*             <code>null</code>.
* @exception  IllegalArgumentException if <code>key</code> is empty.
* @see        #setProperty
* @see        java.lang.SecurityException
* @see        java.lang.SecurityManager#checkPropertyAccess(java.lang.String)
* @see        java.lang.System#getProperties()
public static String getProperty(String key) {
if (key == null) {
throw new NullPointerException("key can't be null");
}
if (key.equals("")) {
throw new IllegalArgumentException("key can't be empty");
}
if (security != null) {
security.checkPropertyAccess(key);
}
return props.getProperty(key);
}
)  ( *
* Gets the system property indicated by the specified key.
* <p>
* First, if there is a security manager, its
* <code>checkPropertyAccess</code> method is called with the
* <code>key</code> as its argument.
* <p>
* If there is no current set of system properties, a set of system
* properties is first created and initialized in the same manner as
* for the <code>getProperties</code> method.
*
* @param      key   the name of the system property.
* @param      def   a default value.
* @return     the string value of the system property,
*             or the default value if there is no property with that key.
*
* @exception  SecurityException  if a security manager exists and its
*             <code>checkPropertyAccess</code> method doesn't allow
*             access to the specified system property.
* @exception  NullPointerException if <code>key</code> is
*             <code>null</code>.
* @exception  IllegalArgumentException if <code>key</code> is empty.
* @see        #setProperty
* @see        java.lang.SecurityManager#checkPropertyAccess(java.lang.String)
* @see        java.lang.System#getProperties()
public static String getProperty(String key, String def) {
if (key == null) {
throw new NullPointerException("key can't be null");
}
if (key.equals("")) {
throw new IllegalArgumentException("key can't be empty");
}
if (security != null) {
security.checkPropertyAccess(key);
}
return props.getProperty(key, def);
}
)  ( *
* Sets the system property indicated by the specified key.
* <p>
* First, if a security manager exists, its
* <code>SecurityManager.checkPermission</code> method
* is called with a <code>PropertyPermission(key, "write")</code>
* permission. This may result in a SecurityException being thrown.
* If no exception is thrown, the specified property is set to the given
* value.
* <p>
*
* @param      key   the name of the system property.
* @param      value the value of the system property.
* @return     the previous value of the system property,
*             or <code>null</code> if it did not have one.
*
* @exception  SecurityException  if a security manager exists and its
*             <code>checkPermission</code> method doesn't allow
*             setting of the specified property.
* @exception  NullPointerException if <code>key</code> is
*             <code>null</code>.
* @exception  IllegalArgumentException if <code>key</code> is empty.
* @see        #getProperty
* @see        java.lang.System#getProperty(java.lang.String)
* @see        java.lang.System#getProperty(java.lang.String, java.lang.String)
* @see        java.util.PropertyPermission
* @see        SecurityManager#checkPermission
* @since      1.2
public static String setProperty(String key, String value) {
if (key == null) {
throw new NullPointerException("key can't be null");
}
if (key.equals("")) {
throw new IllegalArgumentException("key can't be empty");
}
if (security != null)
security.checkPermission(new PropertyPermission(key,
SecurityConstants.PROPERTY_WRITE_ACTION));
return (String) props.setProperty(key, value);
}
)  ( *
* Gets an environment variable. An environment variable is a
* system-dependent external variable that has a string value.
*
* @deprecated The preferred way to extract system-dependent information
*             is the system properties of the
*             <code>java.lang.System.getProperty</code> methods and the
*             corresponding <code>get</code><em>TypeName</em> methods of
*             the <code>Boolean</code>, <code>Integer</code>, and
*             <code>Long</code> primitive types.  For example:
* <blockquote><pre>
*     String classPath = System.getProperty("java.class.path",".");
* <br>
*     if (Boolean.getBoolean("myapp.exper.mode"))
*         enableExpertCommands();
* </pre></blockquote>
*
* @param  name of the environment variable
* @return the value of the variable, or <code>null</code> if the variable
*           is not defined.
* @see    java.lang.Boolean#getBoolean(java.lang.String)
* @see    java.lang.Integer#getInteger(java.lang.String)
* @see    java.lang.Integer#getInteger(java.lang.String, int)
* @see    java.lang.Integer#getInteger(java.lang.String, java.lang.Integer)
* @see    java.lang.Long#getLong(java.lang.String)
* @see    java.lang.Long#getLong(java.lang.String, long)
* @see    java.lang.Long#getLong(java.lang.String, java.lang.Long)
* @see    java.lang.System#getProperties()
* @see    java.lang.System#getProperty(java.lang.String)
* @see    java.lang.System#getProperty(java.lang.String, java.lang.String)
)

: lang.System.getenv§425158400
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL name
   
   \ new statement
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   U" getenv no longer supported, use properties and -D instead:  " COUNT lang.JavaArray.createUnicode§-675323136 DUP §tempvar V!
   name
   OVER  -1461427456 TRUE ( java.lang.String.concat§-1461427456 ) EXECUTE-METHOD
   lang.Error§-32422656.table -906083072 EXECUTE-NEW
   lang.JavaArray.handler§-1096259584
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   
   \ new statement
   0 §break17255 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Returns the same hash code for the given object as
* would be returned by the default method hashCode(),
* whether or not the given object's class overrides
* hashCode().
* The hash code for the null reference is zero.
*
* @param x object for which the hashCode is to be calculated
* @return  the hashCode
* @since   JDK1.1
)

: lang.System.identityHashCode§871524608
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL x
   
   \ new statement
   
   \ new statement
   x A:R@
   R> -1604556800 TRUE ( java.lang.Object.hashCode§-1604556800 ) EXECUTE-METHOD
   0 §break17254 BRANCH
   
   \ new statement
   0 §break17254 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Initialize the system class.  Called after thread initialization.
)

: lang.System.initializeSystemClass§-2071172864
   3 VALLOCATE LOCAL §base0
   
   \ new statement
   0 DUP DUP
   LOCALS fdErr fdIn fdOut |
   
   \ new statement
   \ props = new Properties();
   \ initProperties(props);
   \ sun.misc.Version.init();
   
   
   \ new statement
   io.FileDescriptor.in
   @
   io.FileInputStream§-1165277696.table -1868872192 EXECUTE-NEW
   DUP §base0 V! TO fdIn
   
   \ new statement
   io.FileDescriptor.out
   @
   io.FileOutputStream§1390102016.table 1148470784 EXECUTE-NEW
   DUP 4 §base0 + V! TO fdOut
   
   \ new statement
   io.FileDescriptor.err
   @
   io.FileOutputStream§1390102016.table 1148470784 EXECUTE-NEW
   DUP 8 §base0 + V! TO fdErr
   
   \ new statement
   
   1 VALLOCATE LOCAL §tempvar
   fdIn  " InputStream " CASTTO
   io.BufferedInputStream§1046299136.table 1077559808 EXECUTE-NEW
   DUP §tempvar V!
   " InputStream " CASTTO
   lang.System.setIn0§-1751551232
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   fdOut  " OutputStream " CASTTO
   128
   io.BufferedOutputStream§-505134592.table 140067328 EXECUTE-NEW
   DUP §tempvar V!
   " OutputStream " CASTTO
   TRUE
   io.PrintStream§-1373941760.table 1710182400 EXECUTE-NEW
   DUP §tempvar 4 + V!
   lang.System.setOut0§928150272
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   
   2 VALLOCATE LOCAL §tempvar
   fdErr  " OutputStream " CASTTO
   128
   io.BufferedOutputStream§-505134592.table 140067328 EXECUTE-NEW
   DUP §tempvar V!
   " OutputStream " CASTTO
   TRUE
   io.PrintStream§-1373941760.table 1710182400 EXECUTE-NEW
   DUP §tempvar 4 + V!
   lang.System.setErr0§-129535232
   
   
   
   §tempvar SETVTOP
   
   PURGE 1
   
   
   \ new statement
   \  Load the zip library now in order to keep java.util.zip.ZipFile
   \  from trying to use itself to load this library later.
   \ loadLibrary("zip");
   \  Currently File.deleteOnExit is built on JVM_Exit, which is a
   \  separate mechanism from shutdown hooks. Unfortunately in order to
   \  work properly JVM_Exit implicitly requires that Java signal
   \  handlers be set up for HUP, TERM, and INT (where available). If
   \  File.deleteOnExit were implemented in terms of shutdown hooks this
   \  call to Terminator.setup() could be removed.
   \ Terminator.setup();
   \  Set the maximum amount of direct memory.  This value is controlled
   \  by the vm option -XX:MaxDirectMemorySize=<size>.  This method acts
   \  as an initializer only if it is called before sun.misc.VM.booted().
   \ sun.misc.VM.maxDirectMemory();
   \  Subsystems that are invoked during initialization can invoke
   \  sun.misc.VM.isBooted() in order to avoid doing things that should
   \  wait until the application class loader has been set up.
   \ sun.misc.VM.booted();
   
   
   \ new statement
   0 §break17262 LABEL
   
   \ new statement
   
   
   
   §base0 SETVTOP
   PURGE 4
   
   \ new statement
   DROP
;
( *
* Loads a code file with the specified filename from the local file
* system as a dynamic library. The filename
* argument must be a complete path name.
* <p>
* The call <code>System.load(name)</code> is effectively equivalent
* to the call:
* <blockquote><pre>
* Runtime.getRuntime().load(name)
* </pre></blockquote>
*
* @param      filename   the file to load.
* @exception  SecurityException  if a security manager exists and its
*             <code>checkLink</code> method doesn't allow
*             loading of the specified dynamic library
* @exception  UnsatisfiedLinkError  if the file does not exist.
* @see        java.lang.Runtime#load(java.lang.String)
* @see        java.lang.SecurityManager#checkLink(java.lang.String)
public static void load(String filename) {
Runtime.getRuntime().load0(getCallerClass(), filename);
}
)  ( *
* Loads the system library specified by the <code>libname</code>
* argument. The manner in which a library name is mapped to the
* actual system library is system dependent.
* <p>
* The call <code>System.loadLibrary(name)</code> is effectively
* equivalent to the call
* <blockquote><pre>
* Runtime.getRuntime().loadLibrary(name)
* </pre></blockquote>
*
* @param      libname   the name of the library.
* @exception  SecurityException  if a security manager exists and its
*             <code>checkLink</code> method doesn't allow
*             loading of the specified dynamic library
* @exception  UnsatisfiedLinkError  if the library does not exist.
* @see        java.lang.Runtime#loadLibrary(java.lang.String)
* @see        java.lang.SecurityManager#checkLink(java.lang.String)
public static void loadLibrary(String libname) {
Runtime.getRuntime().loadLibrary0(getCallerClass(), libname);
}
)  ( *
* Maps a library name into a platform-specific string representing
* a native library.
*
* @param      libname the name of the library.
* @return     a platform-dependent native library name.
* @see        java.lang.System#loadLibrary(java.lang.String)
* @see        java.lang.ClassLoader#findLibrary(java.lang.String)
* @since      1.2
public static native String mapLibraryName(String libname);
)  ( *
* The following two methods exist because in, out, and err must be
* initialized to null.  The compiler, however, cannot be permitted to
* inline access to them, since they are later set to more sensible values
* by initializeSystemClass().
)

: lang.System.nullInputStream§-746820096
   LOCAL §exception
   
   \ new statement
   
   \ new statement
   lang.System.currentTimeMillis§1830576896
   DNEGATE D0<
   
   \ new statement
   IF
      
      \ new statement
      0
      DUP 0 V!0 §break17260 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   lang.NullPointerException§561335808.table 966675968 EXECUTE-NEW
   §exception TO §return
   
   \ new statement
   0 §break17260 BRANCH
   
   \ new statement
   0 §break17260 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;


: lang.System.nullPrintStream§-182292992
   LOCAL §exception
   
   \ new statement
   
   \ new statement
   lang.System.currentTimeMillis§1830576896
   DNEGATE D0<
   
   \ new statement
   IF
      
      \ new statement
      0
      DUP 0 V!0 §break17261 BRANCH
      
      \ new statement
   ENDIF
   
   \ new statement
   lang.NullPointerException§561335808.table 966675968 EXECUTE-NEW
   §exception TO §return
   
   \ new statement
   0 §break17261 BRANCH
   
   \ new statement
   0 §break17261 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Runs the finalization methods of any objects pending finalization.
* <p>
* Calling this method suggests that the Java Virtual Machine expend
* effort toward running the <code>finalize</code> methods of objects
* that have been found to be discarded but whose <code>finalize</code>
* methods have not yet been run. When control returns from the
* method call, the Java Virtual Machine has made a best effort to
* complete all outstanding finalizations.
* <p>
* The call <code>System.runFinalization()</code> is effectively
* equivalent to the call:
* <blockquote><pre>
* Runtime.getRuntime().runFinalization()
* </pre></blockquote>
*
* @see     java.lang.Runtime#runFinalization()
)

: lang.System.runFinalization§-1152093696
   
   \ new statement
   
   \ new statement
   \ Runtime.getRuntime().runFinalization();
   
   
   \ new statement
   0 §break17258 LABEL
   
   \ new statement
   
   \ new statement
   DROP
;
( *
* Enable or disable finalization on exit; doing so specifies that the
* finalizers of all objects that have finalizers that have not yet been
* automatically invoked are to be run before the Java runtime exits.
* By default, finalization on exit is disabled.
*
* <p>If there is a security manager,
* its <code>checkExit</code> method is first called
* with 0 as its argument to ensure the exit is allowed.
* This could result in a SecurityException.
*
* @deprecated  This method is inherently unsafe.  It may result in
* 	    finalizers being called on live objects while other threads are
*      concurrently manipulating those objects, resulting in erratic
*	    behavior or deadlock.
* @param value indicating enabling or disabling of finalization
* @throws  SecurityException
*        if a security manager exists and its <code>checkExit</code>
*        method doesn't allow the exit.
*
* @see     java.lang.Runtime#exit(int)
* @see     java.lang.Runtime#gc()
* @see     java.lang.SecurityManager#checkExit(int)
* @since   JDK1.1
)

: lang.System.runFinalizersOnExit§1248555520
   LOCAL value
   
   \ new statement
   
   \ new statement
   \ Runtime.getRuntime().runFinalizersOnExit(value);
   
   
   \ new statement
   0 §break17259 LABEL
   
   \ new statement
   
   PURGE 1
   
   \ new statement
   DROP
;
( *
* Reassigns the "standard" error output stream.
*
* <p>First, if there is a security manager, its <code>checkPermission</code>
* method is called with a <code>RuntimePermission("setIO")</code> permission
*  to see if it's ok to reassign the "standard" error output stream.
*
* @param err the new standard error output stream.
*
* @throws SecurityException
*        if a security manager exists and its
*        <code>checkPermission</code> method doesn't allow
*        reassigning of the standard error output stream.
*
* @see SecurityManager#checkPermission
* @see java.lang.RuntimePermission
*
* @since   JDK1.1
)

: lang.System.setErr§-1753320704
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL err
   
   \ new statement
   
   \ new statement
   lang.System.checkIO§1395811072
   
   \ new statement
   err
   lang.System.setErr0§-129535232
   
   \ new statement
   0 §break17247 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.System.setErr0§-129535232
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL err
   
   \ new statement
   
   \ new statement
   err
   lang.System.err
   OVER 0 V! !
   
   \ new statement
   0 §break17251 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
(  The security manager for the system.
)  \ private static SecurityManager security = null;
( *
* Reassigns the "standard" input stream.
*
* <p>First, if there is a security manager, its <code>checkPermission</code>
* method is called with a <code>RuntimePermission("setIO")</code> permission
*  to see if it's ok to reassign the "standard" input stream.
* <p>
*
* @param in the new standard input stream.
*
* @throws SecurityException
*        if a security manager exists and its
*        <code>checkPermission</code> method doesn't allow
*        reassigning of the standard input stream.
*
* @see SecurityManager#checkPermission
* @see java.lang.RuntimePermission
*
* @since   JDK1.1
)

: lang.System.setIn§-880053504
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL in
   
   \ new statement
   
   \ new statement
   lang.System.checkIO§1395811072
   
   \ new statement
   in
   lang.System.setIn0§-1751551232
   
   \ new statement
   0 §break17245 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.System.setIn0§-1751551232
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL in
   
   \ new statement
   
   \ new statement
   in
   lang.System.in
   OVER 0 V! !
   
   \ new statement
   0 §break17249 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
( *
* Reassigns the "standard" output stream.
*
* <p>First, if there is a security manager, its <code>checkPermission</code>
* method is called with a <code>RuntimePermission("setIO")</code> permission
*  to see if it's ok to reassign the "standard" output stream.
*
* @param out the new standard output stream
*
* @throws SecurityException
*        if a security manager exists and its
*        <code>checkPermission</code> method doesn't allow
*        reassigning of the standard output stream.
*
* @see SecurityManager#checkPermission
* @see java.lang.RuntimePermission
*
* @since   JDK1.1
)

: lang.System.setOut§-970493184
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL out
   
   \ new statement
   
   \ new statement
   lang.System.checkIO§1395811072
   
   \ new statement
   out
   lang.System.setOut0§928150272
   
   \ new statement
   0 §break17246 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;


: lang.System.setOut0§928150272
   1 VALLOCATE LOCAL §base0
   DUP 0 §base0 + V! LOCAL out
   
   \ new statement
   
   \ new statement
   out
   lang.System.out
   OVER 0 V! !
   
   \ new statement
   0 §break17250 LABEL
   
   \ new statement
   
   §base0 SETVTOP
   PURGE 2
   
   \ new statement
   DROP
;
VARIABLE lang.System._staticBlocking
VARIABLE lang.System._staticThread
-1 VALLOCATE CONSTANT lang.System._this
-1 VALLOCATE CONSTANT lang.System.err
-1 VALLOCATE CONSTANT lang.System.in
-1 VALLOCATE CONSTANT lang.System.out

A:HERE VARIABLE lang.System§866276096.table 1 DUP 2* CELLS ALLOT R@ ! A:CELL+
1870680832 R@ ! A:CELL+ lang.System.System§1870680832 VAL R@ ! A:CELL+
A:DROP
