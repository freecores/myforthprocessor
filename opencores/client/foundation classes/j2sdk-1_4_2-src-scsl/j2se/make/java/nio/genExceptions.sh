#! /bin/sh
#
# @(#)genExceptions.sh	1.8 03/01/23
#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Generate exception classes

SPEC=$1
DST=$2

gen() {
  ID=$1
  WHAT=$2
  ARG_TYPE=$3
  ARG_ID=$4
  ARG_PROP=$5
  ARG_PHRASE=$6
  ARG_PARAM="$ARG_TYPE$ $ARG_ID"
  echo '-->' $DST/$ID.java
  out=$DST/${ID}.java

  cat >$out <<__END__
/*
 * @(#)$ID.java	$SCCS_ID $SCCS_DATE
 *
 * Copyright 2000 by Sun Microsystems, Inc.  All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 */

// -- This file was mechanically generated: Do not edit! -- //

package $PACKAGE;


/**$WHAT
 *
 * @version $SCCS_ID, $SCCS_DATE
 * @since $SINCE
 */

public `if [ ${ABSTRACT:-0} = 1 ];
        then echo 'abstract '; fi`class $ID
    extends ${SUPER}
{
__END__

  if [ $ARG_ID ]; then

    cat >>$out <<__END__

    private $ARG_TYPE $ARG_ID;

    /**
     * Constructs an instance of this class. </p>
     *
     * @param  $ARG_ID
     *         The $ARG_PHRASE
     */
    public $ID($ARG_TYPE $ARG_ID) {
        super(String.valueOf($ARG_ID));
	this.$ARG_ID = $ARG_ID;
    }

    /**
     * Retrieves the $ARG_PHRASE. </p>
     *
     * @return  The $ARG_PHRASE
     */
    public $ARG_TYPE get$ARG_PROP() {
        return $ARG_ID;
    }

}
__END__

  else

    cat >>$out <<__END__

    /**
     * Constructs an instance of this class.
     */
    public $ID() { }

}
__END__

  fi
}

. $SPEC
