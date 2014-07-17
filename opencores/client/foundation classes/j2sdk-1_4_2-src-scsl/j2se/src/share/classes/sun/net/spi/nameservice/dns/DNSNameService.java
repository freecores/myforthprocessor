/*
 * @(#)DNSNameService.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.spi.nameservice.dns;

import java.lang.ref.SoftReference;
import java.net.InetAddress;
import java.net.UnknownHostException;
import javax.naming.*;
import javax.naming.directory.*;
import java.util.*;
import sun.net.dns.ResolverConfiguration;
import sun.net.spi.nameservice.*;
import java.security.AccessController;
import sun.security.action.*;

/*
 * A name service provider based on JNDI-DNS.
 */

public final class DNSNameService implements NameService {

    // List of domains specified by property
    private LinkedList domainList = null;

    // JNDI-DNS URL for name servers specified via property
    private String nameProviderUrl = null;

    // Per-thread soft cache of the last temporary context
    private static ThreadLocal contextRef = new ThreadLocal();

    // Simple class to encapsulate the temporary context
    private static class ThreadContext {
	private DirContext dirCtxt;
	private List nsList;

	public ThreadContext(DirContext dirCtxt, List nsList) {
	    this.dirCtxt = dirCtxt;
	    this.nsList = nsList;
	}

	public DirContext dirContext() {
	    return dirCtxt;
	}

	public List nameservers() {
	    return nsList;
	}
    }

    // Returns a per-thread DirContext 
    private DirContext getTemporaryContext() throws NamingException {
	SoftReference ref = (SoftReference)contextRef.get();
	ThreadContext thrCtxt = null;
	List nsList = null;

	// if no property specified we need to obtain the list of servers
	//
	if (nameProviderUrl == null)
	    nsList = ResolverConfiguration.open().nameservers();

	// if soft reference hasn't been gc'ed no property has been
	// specified then we need to check if the DNS configuration
	// has changed. 
	//
	if ((ref != null) && ((thrCtxt = (ThreadContext)ref.get()) != null)) {
	    if (nameProviderUrl == null) {
		if (!thrCtxt.nameservers().equals(nsList)) {
		    // DNS configuration has changed
		    thrCtxt = null;
		}
	    }
	}

	// new thread context needs to be created
	if (thrCtxt == null) {
	    final Properties env = new Properties();
            env.setProperty("java.naming.factory.initial",
                            "com.sun.jndi.dns.DnsContextFactory");

 	    // If no nameservers property specified we create provider URL
	    // based on system configured name servers
	    //
	    String provUrl = nameProviderUrl;
	    if (provUrl == null) {
		provUrl = createProviderURL(nsList);
		if (provUrl.length() == 0) {
		    throw new RuntimeException("bad nameserver configuration");
		}
	    }
	    env.setProperty("java.naming.provider.url", provUrl);

	    // Need to create directory context in privileged block
	    // as JNDI-DNS needs to resolve the name servers.
	    //
	    DirContext dirCtxt;
	    try {
		dirCtxt = (DirContext)
		    java.security.AccessController.doPrivileged(
		 	new java.security.PrivilegedExceptionAction() {
			    public Object run() throws NamingException {
				return new InitialDirContext(env);
			    }
		    });
	    } catch (java.security.PrivilegedActionException pae) {
		throw (NamingException)pae.getException();
	    }

	    // create new soft reference to our thread context
	    //
	    thrCtxt = new ThreadContext(dirCtxt, nsList);
            contextRef.set(new SoftReference(thrCtxt));
        }

        return thrCtxt.dirContext();
    }

    /**
     * Resolves the specified entry in DNS.
     *
     * Canonical name records are recursively resolved (to a maximum
     * of 5 to avoid performance hit and potential CNAME loops).
     *
     * @param	ctx	JNDI directory context
     * @param	name	name to resolve
     * @param	ids	record types to search
     * @param	depth	call depth - pass as 0.
     *
     * @return	array list with results (will have at least on entry)
     *
     * @throws	UnknownHostException if lookup fails or other error.
     */
    private ArrayList resolve(final DirContext ctx, final String name, final String[] ids, 
			      int depth) throws UnknownHostException
    {
        ArrayList results = new ArrayList();
        Attributes attrs;

	// do the query
        try {
            attrs = (Attributes)
                java.security.AccessController.doPrivileged(
                    new java.security.PrivilegedExceptionAction() {
                        public Object run() throws NamingException {
                            return ctx.getAttributes(name, ids);
                        }
                });
        } catch (java.security.PrivilegedActionException pae) {
            throw new UnknownHostException(pae.getException().getMessage());
        }

        // non-requested type returned so enumeration is empty
        NamingEnumeration ne = attrs.getAll();
        if (!ne.hasMoreElements()) {
            throw new UnknownHostException("DNS record not found");
        }

        // iterate through the returned attributes
        UnknownHostException uhe = null;
        try {
            while (ne.hasMoreElements()) {
                Attribute attr = (Attribute)ne.next();
                String attrID = attr.getID();

                for (NamingEnumeration e = attr.getAll(); e.hasMoreElements();) {
                    String addr = (String)e.next();

                    // for canoncical name records do recursive lookup
                    // - also check for CNAME loops to avoid stack overflow

                    if (attrID.equals("CNAME")) {
                        if (depth > 4) {
                            throw new UnknownHostException(name + ": possible CNAME loop");
                        }
                        try {
                            results.addAll(resolve(ctx, addr, ids, depth+1));
                        } catch (UnknownHostException x) {
                            // canonical name can't be resolved.
                            if (uhe == null)
                                uhe = x;
                        }
                    } else {
                        results.add(addr);
                    }
                }
            }
        } catch (NamingException nx) {
            throw new UnknownHostException(nx.getMessage());
        }

        // pending exception as canonical name could not be resolved.
        if (results.isEmpty() && uhe != null) {
            throw uhe;
        }

        return results;
    }

    public DNSNameService() throws Exception {
    
	// default domain 
        String domain =  (String)AccessController.doPrivileged(
            new GetPropertyAction("sun.net.spi.nameservice.domain"));
        if (domain != null && domain.length() > 0) {
            domainList = new LinkedList();
            domainList.add(domain);
        }

	// name servers
	String nameservers
            = (String)AccessController.doPrivileged(
            new GetPropertyAction("sun.net.spi.nameservice.nameservers"));
        if (nameservers != null && nameservers.length() > 0) {
	    nameProviderUrl = createProviderURL(nameservers);
	    if (nameProviderUrl.length() == 0) {
		throw new RuntimeException("malformed nameservers property");
	    }

	} else {

 	    // no property specified so check host DNS resolver configured
	    // with at least one nameserver in dotted notation.
 	    //
	    List nsList = ResolverConfiguration.open().nameservers();
	    if (nsList.size() == 0) 
		throw new RuntimeException("no nameservers provided");
	    boolean found = false;
	    Iterator i = nsList.iterator();
	    while (i.hasNext()) {
		String addr = (String)i.next();
		if (isIPv4LiteralAddress(addr) || isIPv6LiteralAddress(addr)) {
		    found = true;
		    break;
		}
	    }
	    if (!found) {
                throw new RuntimeException("bad nameserver configuration");
            }
	}
    }

    public byte[][] lookupAllHostAddr(String host) throws UnknownHostException {

	// DNS records that we search for
	String[] ids = {"A", "AAAA", "CNAME"};

	// first get directory context
	DirContext ctx;
	try {
	    ctx = getTemporaryContext();
	} catch (NamingException nx) {
	    throw new Error(nx);
	}

	ArrayList results = null;
	UnknownHostException uhe = null;

	// If host already contains a domain name then just look it up
	if (host.indexOf('.') >= 0) {
	    try {
		results = resolve(ctx, host, ids, 0);
	    } catch (UnknownHostException x) {
		uhe = x;
	    }
	}

	// Here we try to resolve the host using the domain suffix or
	// the domain suffix search list. If the host cannot be resolved
	// using the domain suffix then we attempt devolution of
	// the suffix - eg: if we are searching for "foo" and our
	// domain suffix is "eng.sun.com" we will try to resolve
	// "foo.eng.sun.com" and "foo.sun.com".
	// It's not normal to attempt devolation with domains on the
	// domain suffix search list - however as ResolverConfiguration
	// doesn't distinguish domain or search list in the list it
	// returns we approximate by doing devolution on the domain
	// suffix if the list has one entry. 

	if (results == null) {
	    List searchList = null;
	    Iterator i;
	    boolean usingSearchList = false;

	    if (domainList != null) {
		i = domainList.iterator();
	    } else {
		searchList = ResolverConfiguration.open().searchlist();
		if (searchList.size() > 1) {
		    usingSearchList = true;
		}
		i = searchList.iterator();
	    }

	    // iterator through each domain suffix
	    while (i.hasNext()) {
	        String parentDomain = (String)i.next();
		int start = 0;
		while ((start = parentDomain.indexOf(".")) != -1 
		       && start < parentDomain.length() -1) {
		    try {
		 	results = resolve(ctx, host+"."+parentDomain, ids, 0);
			break;
		    } catch (UnknownHostException x) {
			uhe = x;
			if (usingSearchList) {
			    break;
			}

			// devolve
			parentDomain = parentDomain.substring(start+1);
		    }
		}
		if (results != null) {
		    break;
		}
	    }
	}

	// finally try the host if it doesn't have a domain name
        if (results == null && (host.indexOf('.') < 0)) {
            results = resolve(ctx, host, ids, 0);
        }

	// if not found then throw the (last) exception thrown.
        if (results == null) {
	    assert uhe != null;
            throw uhe;
        }

	/**
	 * Convert the array list into a byte aray list - this
	 * filters out any invalid IPv4/IPv6 addresses.
	 */
	assert results.size() > 0;
	byte[][] addrs = new byte[results.size()][];
	int count = 0;
	for (int i=0; i<results.size(); i++) {
	    String addrString = (String)results.get(i);
	    byte addr[] = textToIPv4Address(addrString);
	    if (addr == null) {
		addr = textToIPv6Address(addrString);
	    }
	    if (addr != null) {
		addrs[count++] = addr;
	    }
	}

	/**
  	 * If addresses are filtered then we need to resize the
 	 * array. Additionally if all addresses are filtered then
	 * we throw an exception.
	 */
	if (count == 0) {
	    throw new UnknownHostException(host + ": no valid DNS records");
	}
	if (count < results.size()) {
	    byte[][] tmp = new byte[count][];
	    for (int i=0; i<count; i++) {
		tmp[i] = addrs[i];
	    }	    
	    addrs = tmp;
	}

	return addrs;
    }

    public String getHostByAddr(byte[] addr) throws UnknownHostException {
	try {
	    String literalip = "";
	    if (addr.length == 4) {
		for (int i = addr.length-1; i >= 0; i--) {
		    literalip += (addr[i] & 0xff) +".";
		}
	    } else if (addr.length == 16) {
		for (int i = addr.length-1; i >= 0; i--) {
		    literalip += (addr[i] & 0x0f) +"." +(addr[i] & 0xf0) +".";
		}
	    }
	    if (addr.length == 4) { // ipv4 addr
		literalip += "IN-ADDR.ARPA.";
	    } else if (addr.length == 16) { // ipv6 addr
		literalip += "IP6.INT.";
	    }
	    
	    String[] ids = new String[1];
	    ids[0] = "PTR"; // PTR record

	    DirContext ctx;
	    try {
		ctx = getTemporaryContext();
	    } catch (NamingException nx) {
		throw new Error(nx);
	    }

	    ArrayList results = resolve(ctx, literalip, ids, 0);
	    String host = (String)results.get(0);

	    // remove trailing dot
	    if (host.endsWith(".")) {
                host = host.substring(0, host.length() - 1);
            }
	    return host;
	} catch (Exception e) {
	    throw new UnknownHostException(e.getMessage());
	}
    }    


    // ---------        

    private static void appendIfLiteralAddress(String addr, StringBuffer sb) {
	if (isIPv4LiteralAddress(addr)) {
            sb.append("dns://" + addr + " ");
        } else {
            if (isIPv6LiteralAddress(addr)) {
                sb.append("dns://[" + addr + "] ");
            }
	}
    }

    /*
     * @return String containing the JNDI-DNS provider URL 
     *         corresponding to the supplied List of nameservers.
     */
    private static String createProviderURL(List nsList) {
	Iterator i = nsList.iterator();
	StringBuffer sb = new StringBuffer();
	while (i.hasNext()) {
	    appendIfLiteralAddress((String)i.next(), sb);
	}
	return sb.toString();
    }

    /*
     * @return String containing the JNDI-DNS provider URL 
     *         corresponding to the list of nameservers
     *         contained in the provided str.
     */
    private static String createProviderURL(String str) {
	StringBuffer sb = new StringBuffer();
        StringTokenizer st = new StringTokenizer(str, ",");
        while (st.hasMoreTokens()) {
	    appendIfLiteralAddress(st.nextToken(), sb);
	}
	return sb.toString();
    }

    // ---------
 
    /*
     * @return true if src is a textual representation of an IPv4
     *              address.
     */
    private static boolean isIPv4LiteralAddress(String src) {
	return textToIPv4Address(src) != null;
    }
  
    /*
     * @return true if src is a textual representation of an IPv6
     *              address.
     */
    private static boolean isIPv6LiteralAddress(String src) {
        return textToIPv6Address(src) != null;
    }


    // ----- based on Inet{4,6}Address.textToNumericFormat as we
    // ----- can't get InetAddress to validate the addresses

    /*
     * @return byte array corresponding to the IPv4 address
     *         or null if not a valid IPv4 address.
     */
    private static byte[] textToIPv4Address(String src) {
	if (src.length() == 0) {
	    return null;
	}
	
	int octets;
	char ch;
	byte[] dst = new byte[4];
        char[] srcb = src.toCharArray();
	boolean saw_digit = false;

	octets = 0;
	int i = 0;
	int cur = 0;
	while (i < srcb.length) {
	    ch = srcb[i++];
	    if (Character.isDigit(ch)) {
		int sum =  dst[cur]*10 + (Character.digit(ch, 10) & 0xff);
		
		if (sum > 255)
		    return null;

		dst[cur] = (byte)(sum & 0xff);
		if (! saw_digit) {
		    if (++octets > 4)
			return null;
		    saw_digit = true;
		}
	    } else if (ch == '.' && saw_digit) {
		if (octets == 4)
		    return null;
		cur++;
		dst[cur] = 0;
		saw_digit = false;
	    } else
		return null;
	}
	if (octets < 4)
	    return null;
	return dst;
    }

    /*
     * @return byte array corresponding to the IPv6 address 
     *         or null if not a valid IPv6 address.
     */
    private static byte[] textToIPv6Address(String src) {
	if (src.length() == 0) {
	    return null;
	}

	int colonp;
	char ch;
	boolean saw_xdigit;
	int val;
	char[] srcb = src.toCharArray();
	byte[] dst = new byte[16];

	colonp = -1;
	int i = 0, j = 0;
	/* Leading :: requires some special handling. */
	if (srcb[i] == ':')
	    if (srcb[++i] != ':')
		return null;
	int curtok = i;
	saw_xdigit = false;
	val = 0;
	while (i < srcb.length) {
	    ch = srcb[i++];
	    int chval = Character.digit(ch, 16);
	    if (chval != -1) {
		val <<= 4;
		val |= chval;
		if (val > 0xffff)
		    return null;
		saw_xdigit = true;
		continue;
	    }
	    if (ch == ':') {
		curtok = i;
		if (!saw_xdigit) {
		    if (colonp != -1)
			return null;
		    colonp = j;
		    continue;
		} else if (i == srcb.length) {
		    return null;
		}
		if (j + 2 > 16)
		    return null;
		dst[j++] = (byte) ((val >> 8) & 0xff);
		dst[j++] = (byte) (val & 0xff);
		saw_xdigit = false;
		val = 0;
		continue;
	    }
	    if (ch == '.' && ((j + 4) <= 16)) {
		byte[] v4addr = textToIPv4Address(src.substring(curtok));
		if (v4addr == null) {
		    return null;
		}
		for (int k = 0; k < 4; k++) {
		    dst[j++] = v4addr[k];
		}
		saw_xdigit = false;
		break;	/* '\0' was seen by inet_pton4(). */
	    }
	    return null;
	}
	if (saw_xdigit) {
	    if (j + 2 > 16)
		return null;
	    dst[j++] = (byte) ((val >> 8) & 0xff);
	    dst[j++] = (byte) (val & 0xff);
	}

	if (colonp != -1) {
	    int n = j - colonp;
	    
	    if (j == 16)
		return null;
	    for (i = 1; i <= n; i++) {
		dst[16 - i] = dst[colonp + n - i];
		dst[colonp + n - i] = 0;
	    }
	    j = 16;
	}
	if (j != 16)
	    return null;
 
        return dst;
    }
}
