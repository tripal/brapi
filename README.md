Plant Breeding API Extension module
===================================

[Plant Breeding API](http://www.drupal.org/project/brapi)

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Installation
 * Configuration
 * Authentication & User management
 * Troubleshooting
 * How To Use BrAPI Module
 * Maintainers
 * Sponsors & Partnership


INTRODUCTION
------------

This is an implementation of the Breeding API (http://docs.brapi.apiary.io/) for
Drupal with Tripal module installed.

The Breeding API specifies a standard interface for plant phenotype/genotype
databases to serve their data to crop breeding applications. It is a shared,
open API, to be used by all data providers and data consumers who wish to
participate. Initiated in May 2014, it is currently in an actively developing
state, so now is the time for potential participants to help shape the
specifications to ensure their needs are addressed. The listserve for
discussions and announcements is at [Cornell University][1]. Additional
documentation is in the [Github wiki][2].

[1]: http://mail2.sgn.cornell.edu/cgi-bin/mailman/listinfo/plant-breeding-api
[2]: https://github.com/plantbreeding/documentation


REQUIREMENTS
------------

This module requires the following modules:

 * Tripal 7.x-2.x (not tested under 3.x) (http://www.drupal.org/project/tripal)


INSTALLATION
------------

 * Install as you would normally install a contributed Drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.

 * Enable the module in "Admin menu > Site building > Modules" (/admin/modules).


CONFIGURATION
-------------

 * Configure the CV and example in "Administration > Tripal > Extensions >
   Breeding API > Settings" (/admin/tripal/extension/brapi/configuration).

 * Configure user permissions in "Administration > People > Permissions"
   (/admin/people/permissions):

   - "Use Breeding API": allows users to access to the Breeding API. Roles
     having this permission can not alter data but have read access to all the
     data available through the Breeding API.

   - "Update through Breeding API": allows users to modify database content.
     Roles with this permission can add new entries and update or remove exiting
     ones.

   - "Administer Breeding API": allows users to change the Breeding API settings
     such as the CV term uses and the default entries to use as example.


AUTHENTICATION
--------------

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


TROUBLESHOOTING
---------------

Please note that this module supports a limited number of fields that are part
of the Breeding API specifications.
If the Breeding API does not display all the data you expect for a given entry,
make sure you associated the appropriate Chado CV term with the fields managed
by this module. Your data should also be stored in Chado the way the Breeding
API Drupal module expects it to be. If you store things differently, consider
also storing them the way this modules expect them to be stored.


HOW TO USE BRAPI MODULE
-----------------------

Install the module, enable it and adjust the CV settings according to your Chado
database instance and the way you store your biological data.
You can query the BrAPI service through the URL
http://<your-drupal-site>/brapi/v1/<service-name>

To test your instance, you can use the RestClient plugin for your favorite web
browser client from http://restclient.net/.


MAINTAINERS
-----------

Current maintainers:

 * Valentin Guignon (vguignon) - https://www.drupal.org/user/423148


SPONSORS & PARTNERSHIP
----------------------
The Breeding API Drupal implementation has been sponsored by Bioversity
International, a CGIAR Research Centre.
The Breeding API project has been sponsored by the Bill and Melinda Gates
Foundation which funded the breeding API hackathon in June 2015 in Seattle and
in July 2016 in Ithaca.

Partners of the Breeding API project are:
Bioversity International     http://www.bioversityinternational.org/
BMS               http://www.integratedbreeding.net/breeding-management-system/
BTI (Cassavabase, Musabase)  http://bti.cornell.edu/
CIMMYT                       http://www.cimmyt.org/
CIP                          http://cipotato.org/
CIRAD                        http://www.cirad.fr/
GOBII Porject                http://gobiiproject.org/
IRRI                         http://irri.org/
The James Hutton Institute   http://www.hutton.ac.uk/
WUR                          http://www.wur.nl/
