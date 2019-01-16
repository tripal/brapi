
Authentication
================

The Breeding API Drupal module relies on Drupal user management system to manage
its users. Users can authenticate either through the breeding API
(https://<your-drupal-site>/brapi/v1/token POST call) or though the Drupal
interface (/user). The Drupal user management interface can be used to add or
remove users. Users can update their account properties (name, e-mail,...) using
Drupal user interface as well. User access levels are managed using Drupal roles
and permission system (see "Configuration" paragraph).

Authentication process and calls that require authentication MUST go through the
HTTPS protocol (ie HTTP Secure through SSL or TLS encryption). The
authentication token must be provided with each BrAPI call that requires
authentication (but it can be omitted in other cases). It should[*] be provided
by the client application through the HTTP header field "Authorization" and must
be of the form:

Authorization: Bearer SESS<some codes>=<some other codes>

Note that "SESS<some codes>" part correspond to the Drupal user session cookie
name and "<some other codes>" correspond to the cookie value.
[*]: as the module relies on Drupal user management system, the token can be
replaced by Drupal regular session cookie. BrAPI client applications that
support cookies but not authentication token will support (non-standard)
authentication with this BrAPI implementation.
