Plant Breeding API Tripal Extension module
============================================

This is an implementation of the Breeding API (http://docs.brapi.apiary.io/) for
Drupal with [Tripal](https://github.com/tripal/tripal) installed.

The Breeding API specifies a standard interface for plant phenotype/genotype
databases to serve their data to crop breeding applications. It is a shared,
open API, to be used by all data providers and data consumers who wish to
participate. Initiated in May 2014, it is currently in an actively developing
state, so now is the time for potential participants to help shape the
specifications to ensure their needs are addressed. The listserve for
discussions and announcements is at [Cornell University][1]. Additional
documentation is in the [Github wiki][2]. The latest up-to-date specifications
and discussions can be found on the [git repository][3] and the [issue queue][4].

[1]: http://mail2.sgn.cornell.edu/cgi-bin/mailman/listinfo/plant-breeding-api
[2]: https://github.com/plantbreeding/documentation
[3]: https://github.com/plantbreeding/API
[4]: https://github.com/plantbreeding/API/issues


REQUIREMENTS
------------

This module requires the following modules:

 * Drupal 7.x
 * Tripal 7.x-2.x (3.x not tested yet) (http://www.drupal.org/project/tripal)


QUICK START
------------

 * Install as you would normally install a contributed Drupal module. See:
   https://drupal.org/documentation/install/modules-themes/modules-7
   for further information.

 * Enable the module in "Admin menu > Site building > Modules" (path 
   /admin/modules).

 * In case you have an obsolete version of the MCPD vocabulary, you may need to
   update it using the button "Reload Chado MCPD CV" in the MCPD Settings
   section of admin/tripal/extension/brapi/configuration page.

 * Adjust the CV settings according to your Chado database instance and the way 
   you store your biological data. You may also setup germplasm attribute settings, 
   add some external BrAPI site references and setup call aggregation if needed.

USAGE
-------

You can query the BrAPI service through the URL
http(s)://<your-drupal-site>/brapi/v1/<service-name>?<query-parameters>

Ex.:
https://www.crop-diversity.org/mgis/brapi/v1/calls

It will return a JSON structure that can be processed by BrAPI-compliant
applications.

Testing:
To test your instance, you can either use the RestClient plugin for your
favorite web browser client from http://restclient.net/ or use the provided
BrAPI query interface (path brapi/query). Note that when you use this interface
as admin, you will have an additional checkbox which can enable debug mode.

**Javascript & Dynamic HTML**

If you want to add BrAPI fields on your pages that should be automatically and
dynamically populated using external BrAPI sites, you can use the following HTML
snippet:

```
  <form class="brapi-autoquery"
    action="https://BRAPI_SERVER/brapi/v1/SERVICE?PARAMETERS..." method="GET">
    <input type="hidden" name="brapi_html" value="URL_ENCODED_HTML_STRING"/>
    <input type="submit" name="submit" value="Get BrAPI data"/>
  </form>
```

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

DOCUMENTATION
---------------

There is extensive documentation for this module hosted on ReadtheDocs at
https://brapi.readthedocs.io/en/latest/. This includes documentation for
installing, using and extending this module.


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
