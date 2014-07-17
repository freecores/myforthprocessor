#
# Copyright 2002 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)rpm-jre.spec	1.20 03/02/19
#
%prep
%ifos Linux
%ifarch %{arch}
if [ ! -d /usr/src/redhat/RPMS/${arch} ]; then mkdir -p /usr/src/redhat/RPMS/${arch}; fi
if [ ! -d /usr/src/redhat/SRPMS ]; then mkdir -p /usr/src/redhat/SRPMS; fi

%setup -n %{jre_name}%{jre_version}

%build

%install
if [ -d $RPM_BUILD_ROOT/%{jre_prefix}/%{jre_name}%{jre_version} ]; then 
   rm -rf $RPM_BUILD_ROOT/%{jre_prefix}/%{jre_name}%{jre_version}
fi
mkdir -p $RPM_BUILD_ROOT%{jre_prefix}/%{jre_name}%{jre_version}
cp -dpr $RPM_BUILD_DIR/%{jre_name}%{jre_version}/* $RPM_BUILD_ROOT/%{jre_prefix}/%{jre_name}%{jre_version}/


%files
%ifarch ia64
%defattr(-,root,root) 
%else
%defattr(-,-,bin) 
%endif
%{jre_prefix}/%{jre_name}%{jre_version}/*

%clean
rm -rf $RPM_BUILD_DIR/%{jre_name}%{jre_version}
rm -rf $RPM_BUILD_ROOT%{jre_prefix}/%{jre_name}%{jre_version}


%else
echo "This package is for %{arch}."
echo "To override add the "--ignorearch" option"
%endif

%else
echo "This package is for the Linux operating system."
echo "To override add the /"--ignoreos/" option"
%endif

%post
#Returns the instpath of the latest javaws or ""
#With linux we could have both j2re and sdk installed.
#Therefore we choose the jre javaws over the one with sdk.

GetLatestJavaWSPath() {
   tfile=/tmp/getjrelist$$.tmp
   touch $tfile

       jre_list=`ls -1d %{jre_prefix}/%{jre_name}* 2>/dev/null`
       for i in $jre_list; do
           if [ -d $i/javaws ]; then
               echo $i | awk -v JRE_NAME=%{jre_prefix}/%{jre_name} '{ \
               split($1,vers,JRE_NAME); \
	       #We add weight 1 to prefer jre over sdk
	       printf("%s\t%s\t%s/javaws\n",vers[2],"1",$1) \
		}' >> $tfile
	   fi
       done	
       jdk_list=`ls -1d %{jdk_prefix}/%{jdk_name}* 2>/dev/null`
       for i in $jdk_list; do
          if [ -d $i/jre/javaws ]; then
             echo $i | awk -v JDK_NAME=%{jdk_prefix}/%{jdk_name} '{ \
             split($1,vers,JDK_NAME); \
             #We add weight 2 to prefer jre over jdk
             printf("%s\t%s\t%s/jre/javaws\n",vers[2],"2",$1) \
         }' >> $tfile
          fi
       done

   #We prefer  to use j2re vs. jdk by reverse ordering the first field 
   JAVAWS_PATH=""
   JAVAWS_PATH=`sort -k1,1r < $tfile | head -1 | awk '{print $3}'`
   LATEST_JAVAWS_PATH=$JAVAWS_PATH/javaws
   rm -f $tfile
}
# End of GetLatestJavaWSPath
UNPACK_CMD=%{jre_prefix}/%{jre_name}%{jre_version}/lib/unpack
if [ -f $UNPACK_CMD ]; then 
   chmod +x $UNPACK_CMD
   PACKED_JARS=%{packed_jars}
   for i in $PACKED_JARS; do
	srcFile=%{jre_prefix}/%{jre_name}%{jre_version}/`dirname $i`/`basename $i .jar`.pack
	dstFile=%{jre_prefix}/%{jre_name}%{jre_version}/$i
	$UNPACK_CMD $srcFile $dstFile 
	if [  ! -f  $dstFile ]; then
	    printf "Error: unpack could not create %s. Please refer to the Troubleshooting" $dstFile 
	    printf "Section of the Installation Instructions on the download page."
	    exit 1
	fi
	rm -f  $srcFile
   done
   rm -f $UNPACK_CMD
else 
   printf "Error: unpack command could not be found. Please refer to the "
   printf "TroubleShooting Section of the Installation Instructions on "
   printf "the download page."
   printf "Please do not attempt to install this archive file.\n"
   exit 2
fi
if [ ! -d /usr/local/man/man1 ]; then mkdir -p /usr/local/man/man1; fi
cp -dpr %{jre_prefix}/%{jre_name}%{jre_version}/man/man1/* %{jre_manpath}
userid=`expr "\`id\`" : ".*uid=[0-9]*(\(.[0-9a-z]*\)) .*"`
PREFS_LOCATION=%{jre_prefix}/%{jre_name}%{jre_version} 
if [ x$userid = "xroot" ] ; then
  PREFS_LOCATION=/etc/.java
fi
if [ ! -d $PREFS_LOCATION ] ; then
  mkdir -m 755 $PREFS_LOCATION
fi
if [ ! -d $PREFS_LOCATION/.systemPrefs ] ; then
  mkdir -m 755 $PREFS_LOCATION/.systemPrefs
fi
if [ ! -f $PREFS_LOCATION/.systemPrefs/.system.lock ] ; then
  touch $PREFS_LOCATION/.systemPrefs/.system.lock
  chmod 644 $PREFS_LOCATION/.systemPrefs/.system.lock
fi
if [ ! -f $PREFS_LOCATION/.systemPrefs/.systemRootModFile ] ; then
  touch $PREFS_LOCATION/.systemPrefs/.systemRootModFile
  chmod 644 $PREFS_LOCATION/.systemPrefs/.systemRootModFile
fi
%ifarch ia64
# No javaws on 64-bit
%else
MIME_TYPE=%{mime_type}
JAVAWS_BIN=%{javaws_bin}

if [ x$userid = "xroot" ] ; then
    MAILCAP_FILE=$RPM_BUILD_ROOT/etc/mailcap
    MIME_FILE=$RPM_BUILD_ROOT/etc/mime.types
else
    MAILCAP_FILE=$HOME/.mailcap
    MIME_FILE=$HOME/.mime.types
fi
GetLatestJavaWSPath
if [ \( -n "${LATEST_JAVAWS_PATH}" -a -x "${LATEST_JAVAWS_PATH}" \) ]; then
    if [ -w $MAILCAP_FILE ]; then
       jGrep=`grep -n ${MIME_TYPE} $MAILCAP_FILE`      
       if [ -n "$jGrep" ] ; then
          # Remove existing entry  - so we are sure the path is right
          grep -v ${MIME_TYPE} < $MAILCAP_FILE | grep -v "# Java Web Start" > /tmp/.mailcap1
          cp -p /tmp/.mailcap1 $MAILCAP_FILE
	  rm /tmp/.mailcap1
     fi
   # Doesn't contain application/x-java-jnlp-file file, add it.
   echo "# Java Web Start" >> $MAILCAP_FILE
   echo "${MIME_TYPE}; $LATEST_JAVAWS_PATH %s" >> $MAILCAP_FILE
   elif [ -w `dirname $MAILCAP_FILE` ]; then
       # mailcap doesn't exist, create it.
       echo "# Java Web Start" > $MAILCAP_FILE
       echo "${MIME_TYPE}; $LATEST_JAVAWS_PATH %s" >> $MAILCAP_FILE
       chmod 644 $MAILCAP_FILE
    else
        printf "Cannot write to file %s check permissions" $MAILCAP_FILE
    fi
    if [ -w $MIME_FILE ]; then
      # the file exists, make sure it contains jnlp
      jGrep=`grep ${MIME_TYPE} $MIME_FILE`
      if [ -z "${jGrep}" ]; then
        # doesn't contain Javaws, add it
        echo "type=${MIME_TYPE} desc=\"Java Web Start\" exts=\"jnlp\"" >> \
         $MIME_FILE
      fi
    elif [ -w `dirname $MIME_FILE` ]; then
      # Doesn't exist so create it, even though if it doesn't exist it
      # implies you are not using Communicator, but we'll create it on the
      # chance that you do install Communicator.
      echo '#--Netscape Communications Corporation MIME Information' > \
       $MIME_FILE
      echo '#Do not delete the above line. It is used to identify the file type.' \
   >> $MIME_FILE
      echo '#mime types added by Netscape Helper' >> $MIME_FILE
      echo 'type=application/x-java-jnlp-file desc="Java Web Start" exts="jnlp"' >> \
   $MIME_FILE
      chmod 644 $MIME_FILE
    else
        printf "Cannot write to file %s check permissions" $MAILCAP_FILE
    fi
fi
#
#copy sun_java.desktop file to /usr/share/applications directory,
#since this is the default location for all desktop files starting with
#GNOME 2.0
#
DESKTOP_FILE_PATH=`echo $DESKTOP_FILE_PATH`
if [ "${DESKTOP_FILE_PATH}" = "" ]; then 
    DESKTOP_FILE_PATH=/usr/share/applications
    if [ ! -d $DESKTOP_FILE_PATH ] ; then
       DESKTOP_FILE_PATH=/usr/share/gnome/apps/Settings
       if [ ! -d $DESKTOP_FILE_PATH ] ; then
           mkdir -p $DESKTOP_FILE_PATH
       fi
    fi
fi
if [ -f ${DESKTOP_FILE_PATH}/sun_java.desktop ]; then
  rm ${DESKTOP_FILE_PATH}/sun_java.desktop
fi
INSTALL_PATH=%{jre_prefix}
JRE_STRING=%{jre_name}%{jre_version}
sed "s|INSTALL_DIR|$INSTALL_PATH|g"  %{jre_prefix}/%{jre_name}%{jre_version}/plugin/desktop/sun_java.desktop | sed "s|JRE_NAME_VERSION|$JRE_STRING|g" > ${DESKTOP_FILE_PATH}/sun_java.desktop
%endif

%preun
if [ ! -d %{jre_prefix}/%{jre_name}%{jre_version}/man/man1 ]; then mkdir -p %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
if [ -f %{jre_manpath}/java.1 ]; then mv -f %{jre_manpath}/java.1 %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
if [ -f %{jre_manpath}/keytool.1 ]; then mv -f %{jre_manpath}/keytool.1 %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
if [ -f %{jre_manpath}/rmid.1 ]; then mv -f %{jre_manpath}/rmid.1 %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
if [ -f %{jre_manpath}/rmiregistry.1 ]; then mv -f %{jre_manpath}/rmiregistry.1 %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
if [ -f %{jre_manpath}/tnameserv.1 ]; then mv -f %{jre_manpath}/tnameserv.1 %{jre_prefix}/%{jre_name}%{jre_version}/man/man1; fi
PACKED_JARS=%{packed_jars}
UNPACK_CMD=%{jre_prefix}/%{jre_name}%{jre_version}/lib/unpack
for i in $PACKED_JARS; do
   touch %{jre_prefix}/%{jre_name}%{jre_version}/`dirname $i`/`basename $i .jar`.pack
   rm -f %{jre_prefix}/%{jre_name}%{jre_version}/$i
done
touch $UNPACK_CMD 

%postun
#Returns the instpath of the latest javaws or ""
#With linux we could have both j2re and sdk installed.
#Therefore we choose the jre javaws over the one with sdk.

GetLatestJavaWSPath() {
   tfile=/tmp/getjrelist$$.tmp
   touch $tfile

       jre_list=`ls -1d %{jre_prefix}/%{jre_name}* 2>/dev/null`
       for i in $jre_list; do
           if [ -d $i/javaws ]; then
               echo $i | awk -v JRE_NAME=%{jre_prefix}/%{jre_name} '{ \
               split($1,vers,JRE_NAME); \
	       #We add weight 1 to prefer jre over sdk
	       printf("%s\t%s\t%s/javaws\n",vers[2],"1",$1) \
		}' >> $tfile
	   fi
       done	
       jdk_list=`ls -1d %{jdk_prefix}/%{jdk_name}* 2>/dev/null`
       for i in $jdk_list; do
          if [ -d $i/jre/javaws ]; then
             echo $i | awk -v JDK_NAME=%{jdk_prefix}/%{jdk_name} '{ \
             split($1,vers,JDK_NAME); \
             #We add weight 2 to prefer jre over jdk
             printf("%s\t%s\t%s/jre/javaws\n",vers[2],"2",$1) \
         }' >> $tfile
          fi
       done

   #We prefer  to use j2re vs. jdk by reverse ordering the first field 
   JAVAWS_PATH=""
   JAVAWS_PATH=`sort -k1,1r < $tfile | head -1 | awk '{print $3}'`
   LATEST_JAVAWS_PATH=$JAVAWS_PATH/javaws
   rm -f $tfile
}
# End of GetLatestJavaWSPATH
rm -rf %{jre_prefix}/%{jre_name}%{jre_version} 
%ifarch ia64
# No javaws on 64-bit
%else
MIME_TYPE=%{mime_type}
userid=`expr "\`id\`" : ".*uid=[0-9]*(\(.[0-9a-z]*\)) .*"`

if [ x$userid = "xroot" ] ; then
    MAILCAP_FILE=$RPM_BUILD_ROOT/etc/mailcap
    MIME_FILE=$RPM_BUILD_ROOT/etc/mime.types
else
    MAILCAP_FILE=$HOME/.mailcap
    MIME_FILE=$HOME/.mime.types
fi
if [ -w $MAILCAP_FILE ]; then
  jGrep=`grep -n ${MIME_TYPE} $MAILCAP_FILE`
  if [ -n "$jGrep" ]; then
    grep -v ${MIME_TYPE} < $MAILCAP_FILE | grep -v "# Java Web Start" > /tmp/.mailcap1
    cp -p /tmp/.mailcap1 $MAILCAP_FILE
    rm /tmp/.mailcap1
  fi
else  
  printf "Cannot write to file %s check permissions" $MAILCAP_FILE
fi
if [ -w $MIME_FILE ]; then
  jGrep=`grep -n ${MIME_TYPE} $MIME_FILE`
  if [ -n "$jGrep" ]; then
    grep -v ${MIME_TYPE} < $MIME_FILE > /tmp/.mime.types1
    cp -p /tmp/.mime.types1 $MIME_FILE
    rm /tmp/.mime.types1
  fi
else
    printf "Cannot write to file %s check permissions" $MIME_FILE
fi
# Try to set latest path to Java Web Start
GetLatestJavaWSPath
if [ \( -n "${LATEST_JAVAWS_PATH}" -a -x "${LATEST_JAVAWS_PATH}" \) ]; then
    if [ -w $MAILCAP_FILE ]; then
       jGrep=`grep -n ${MIME_TYPE} $MAILCAP_FILE`      
       if [ -n "$jGrep" ] ; then
          # Remove existing entry  - so we are sure the path is right
          grep -v ${MIME_TYPE} < $MAILCAP_FILE | grep -v "# Java Web Start" > /tmp/.mailcap1
          cp -p /tmp/.mailcap1 $MAILCAP_FILE
	  rm /tmp/.mailcap1
     fi
   # Doesn't contain application/x-java-jnlp-file file, add it.
   echo "# Java Web Start" >> $MAILCAP_FILE
   echo "${MIME_TYPE}; $LATEST_JAVAWS_PATH %s" >> $MAILCAP_FILE
    elif [ -w `dirname $MAILCAP_FILE` ]; then
       # mailcap doesn't exist, create it.
       echo "# Java Web Start" > $MAILCAP_FILE
       echo "${MIME_TYPE}; $LATEST_JAVAWS_PATH %s" >> $MAILCAP_FILE
       chmod 644 $MAILCAP_FILE
    else
        printf "Cannot write to file %s check permissions" $MAILCAP_FILE
    fi
    if [ -w $MIME_FILE ]; then
      # the file exists, make sure it contains jnlp
      jGrep=`grep ${MIME_TYPE} $MIME_FILE`
      if [ -z "${jGrep}" ]; then
        # doesn't contain Javaws, add it
        echo "type=${MIME_TYPE} desc=\"Java Web Start\" exts=\"jnlp\"" >> \
         $MIME_FILE
      fi
    elif [ -w `dirname $MIME_FILE` ]; then
      # Doesn't exist so create it, even though if it doesn't exist it
      # implies you are not using Communicator, but we'll create it on the
      # chance that you do install Communicator.
      echo '#--Netscape Communications Corporation MIME Information' > \
       $MIME_FILE
      echo '#Do not delete the above line. It is used to identify the file type.' \
   >> $MIME_FILE
      echo '#mime types added by Netscape Helper' >> $MIME_FILE
      echo 'type=application/x-java-jnlp-file desc="Java Web Start" exts="jnlp"' >> \
   $MIME_FILE
      chmod 644 $MIME_FILE
    else
        printf "Cannot write to file %s check permissions" $MAILCAP_FILE
    fi
fi
#
# remove sun_java.desktop file from common desktop file location 
# if it belongs to this jre.
#
JRE_VERSION=%{jre_version}
DESKTOP_FILE_PATH=`echo $DESKTOP_FILE_PATH`
if [ "${DESKTOP_FILE_PATH}" = "" ]; then 
    DESKTOP_FILE_PATH=/usr/share/applications
    if [ ! -d $DESKTOP_FILE_PATH ] ; then
       DESKTOP_FILE_PATH=/usr/share/gnome/apps/Settings
    fi
fi
FOUND_CNT=`grep -c $JRE_VERSION/ $DESKTOP_FILE_PATH/sun_java.desktop`
if [ `expr $FOUND_CNT \>= 1` -eq 1 ]; then 
   rm -f $DESKTOP_FILE_PATH/sun_java.desktop
fi
%endif
