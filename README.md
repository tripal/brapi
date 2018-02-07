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
 * Extending BrAPI Module: Hooks
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
documentation is in the [Github wiki][2]. The latest up-to-date specifications
and discussions can be found on the git repository [3] and the issue queue [4].

[1]: http://mail2.sgn.cornell.edu/cgi-bin/mailman/listinfo/plant-breeding-api
[2]: https://github.com/plantbreeding/documentation
[3]: https://github.com/plantbreeding/API
[4]: https://github.com/plantbreeding/API/issues


REQUIREMENTS
------------

This module requires the following modules:

 * Tripal 7.x-2.x (3.x not tested yet) (http://www.drupal.org/project/tripal)


INSTALLATION
------------

 * Install as you would normally install a contributed Drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.

 * Enable the module in "Admin menu > Site building > Modules" (path 
   /admin/modules).

 * In case you have an obsolete version of the MCPD vocabulary, you may need to
   update it using the button "Reload Chado MCPD CV" in the MCPD Settings
   section of admin/tripal/extension/brapi/configuration page.


CONFIGURATION
------------- 

 * Configure BrAPI in "Administration > Tripal > Extensions > Breeding API >
   Settings" (path /admin/tripal/extension/brapi/configuration). The settings
   are organized by sections:
   
   - "Example value settings": you can specify here which identifiers should be
     used to demo the calls on the overview page (path brapi/overview). The
     identifiers correspond to the names inside curly brackets used in call
     URLs. These settings are optional and do not impact BrAPI behaviour.

   - "Storage options": this is the place where you specify how you store your
     data in Chado and where BrAPI could find it. For instance, if you don't
     store the common crop name of your stock in the stockprop table, then BrAPI
     can use the organism table instead. Therefore, you will change the "Common
     crop name storage" settings from "Stored in stockprop table" to "Stored
     in organism table". BrAPI module will find the common crop name using
     stock.organism_id --> organism table --> organism.common_name.
     However, if you use the stockprop table, then you must ensure you also
     setup the appropriate cvterm_id for "commonCropName" in the MCPD settings.
     All the other parameters have a similar approaches.
     The "Date storage format" specifies the way your dates are stored and not
     the way BrAPI will display them (which is in the specifications). As dates
     are stored as strings in the value field of property, they can be stored in
     a human-readable manner as well as in timestamp format.
   
   - "Controlled vocabulary settings": this is where you associate BrAPI terms
     used by BrAPI calls to corresponding CV terms available in Chado and used
     by the corresponding field (*prop.type_id or *_cvterm.cvterm_id). Use the
     auto-completion feature to find the corresponding terms from the
     appropriate CV. Make sure the numeric value of the identifier of each
     cvterm (cvterm_id) is present; the term name can be omitted. In some cases,
     you may specify several CV terms by separating them with comma.
   
   - "MCPD settings": BrAPI uses MCPD (Multi-Crop Passport Descriptor controlled
     vocabulary). If you use your own terms rather than the MCPD ones, you can
     override these settings here the same way you did for "Controlled
     vocabulary settings".
     
   - "Germplasm attribute settings": these settings hold attribute categories
     and germplasm attributes stored in your Chado database. Category names are
     case sensitive: make sure you always use the same case. For each attribute
     you define, you can specify either the Chado CV (cv_id) that holds all the
     terms used by the attribute or a list of CV terms (cvterm_id) used. The
     BrAPI system will look for attributes either in the stockprop table if they
     have a type_id corresponding to the selected CV terms and in stock_cvterm
     table if the have a corresponding cvterm_id. For stockprop, the value field
     will be returned as the attribute value while for the stock_cvterm table,
     the CV term name (cvterm.name) will be returned as value.
     Only the attributes that you define here will be exposed by BrAPI.
     
    - "Call aggregation options": it is where you can setup call aggregation or
    proxy-ing. "Call aggregation" is a way to complete missing fields (null)
    returned by a given local BrAPI call with field values provided by the same
    call run on other BrAPI instances. Proxy-ing is relaying a BrAPI call to
    another BrAPI instance and serve its result to the client without having
    hime/her to know about the other BrAPI instance: the client only sees one
    BrAPI en point which make his/her life easier. With this BrAPI module, you
    can select which call you wish to aggregate with which other BrAPI
    instances. If you don't provide any data for a given call which is
    aggregated, all the field values from the other instance will be used,
    which is basically "proxy-ing" the call.
    In order to use aggregation, you need first to record external BrAPI sites
    in the system. To do so, you will have to go to the "Content > BrAPI Site
    References" page (path admin/content/brapi_site) and add new references.
    Then you can go back to the BrAPI setting page and will be able to select
    one of your referenced BrAPI sites for the call you would like to aggregate
    or proxy.

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
database instance and the way you store your biological data. You may also setup
germplasm attribute settings, add some external BrAPI site references and setup
call aggregation if needed.

You can query the BrAPI service through the URL
http(s)://<your-drupal-site>/brapi/v1/<service-name>?<query-parameters>

Ex.:
https://www.crop-diversity.org/mgis/brapi/v1/calls

It will return a JSON structure that can be processed by BrAPI-compliant
applications.

Tests:
To test your instance, you can either use the RestClient plugin for your
favorite web browser client from http://restclient.net/ or use the provided
BrAPI query interface (path brapi/query). Note that when you use this interface
as admin, you will have an additional checkbox which can enable debug mode.

Javascript & dynamic HTML:
If you want to add BrAPI fields on your pages that should be automatically and
dynamically populated using external BrAPI site, you can use the following HTML
snippet:

  <form class="brapi-autoquery" action="https://BRAPI_SERVER/brapi/v1/SERVICE?PARAMETERS..." method="GET">
    <input type="hidden" name="brapi_html" value="URL_ENCODED_HTML_STRING"/>
    <input type="submit" name="submit" value="Get BrAPI data"/>
  </form>

where "BRAPI_SERVER" is the BrAPI server name, "SERVICE?PARAMETERS..." is the
BrAPI service to query with its optional parameters and values and 
"URL_ENCODED_HTML_STRING" is the URL-encoded HTML code to use to replace the
form. In this string, not encoded place-holder string will be replaced by
properties of the (first) JSON object returned. A place-holder is a the property
name as described in the BrAPI specs inside square-brackets.
For instance "[germplasmName]" (for the "germplasm-search" call) will be
replace by the germplasm name of the first germplasm returned by the call.
Note: array or object properties can not be used here.
The form can contain additional call parameters using hidden input or select
fields wrapped by an HTML element having the CSS class
"brapi-query-filter-post"


EXTENDING BRAPI MODULE: HOOKS
-----------------------------

* hook_brapi_cv_settings_alter(&settings): allows other module to alter CV
  settings.
  See also brapi_get_cv_settings() documentation in api/brapi.api.inc.
  
* hook_brapi_data_mapping_alter(&$brapi_data_mapping): allows other module to
  alter BrAPI data mapping settings.
  See also brapi_get_data_mapping() documentation in api/brapi.const.inc.
  
* hook_brapi_CALL_FUNC_NAME_alter(&$data, &$context): allows other module to
  alter the result of a BrAPI call. $data contains the result structure
  currently returned by the call and $context contains the metadata and debug
  strings.
  See also brapi_call_wrapper() documentation in api/brapi.calls.inc.

* hook_brapi_CALL_FUNC_NAME_brapi_error_alter(&$output): allows other module to
  alter error raised by BrAPI (BrAPI exceptions associated to specific HTTP
  error codes (like 400 bad request, 404 not found, 501 not implemented and
  such). It is also possible to replace errors by results since $output contains
  the full JSON structure returned by BrAPI. This may be useful for
  unimplemented calls raising 501 errors or values not found by current
  implementation raising 404 if your module can handle those.
  See also brapi_call_wrapper() documentation in api/brapi.calls.inc.

* hook_brapi_CALL_FUNC_NAME_error_alter(): allows other module to alter other
  type of errors (not raised by BrAPI, typically PHP exceptions).
  See also brapi_call_wrapper() documentation in api/brapi.calls.inc.


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
