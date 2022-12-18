Plant Breeding API server
*************************

Plant Breeding API server implementation for Drupal.

===============================

CONTENTS OF THIS FILE
---------------------

 * Introduction
 * Requirements
 * Installation
 * Configuration
 * Administration
 * Maintainers

INTRODUCTION
------------

Version 4.0.x is a complete rewrite of the BrAPI module for Drupal 8+. This
module only contains the BrAPI server part. For a Drupal BrAPI client, see
External Entities BrAPI Storage plugin (
https://www.drupal.org/project/xnttbrapi/).

You can configure wich version of BrAPI your site support (1.x and/or 2.x).
To enable a call, you need first to map all the data types it is using to Drupal
content types. Then, you can enable the call.
Each data type is related to a given BrAPI version and mappings are also
version related.

BrAPI calls are handled through:
/brapi/v<1 or 2>/<call name with arguments>[parameters such as: ?page=0&pageSize=10]

Current beta support:
-calls for listing and displaying a single element (read only)
-search calls
-pager
-filters
-authentication
-permissions are not fully managed (read only)
-only JSON output


REQUIREMENTS
------------

none.

INSTALLATION
------------

 * Install as you would normally install a contributed Drupal module. Visit
   https://www.drupal.org/node/1897420 for further information.

CONFIGURATION
-------------

Got to your site /brapi/admin/ page. From there, you can manage supported BrAPI
version, data types mapping (/brapi/admin/datatypes) and enabled calls
(/brapi/admin/calls).
On the "people" > "permissions" page, got to the "Plant Breeding API" section
and set the "Use BrAPI" permission to the appropriate roles. You may grant that
permission to the "Anonymous" role for anonymous access.


ADMINISTRATION
--------------

* To manage user, use Drupal native user management interface.

* To get a user access token, login to the website using the user account and
  visit `/brapi/token/new`.
  
* To clear search cache, use drush command:
```
drush cc bin brapi_search
```



MAINTAINERS
-----------

Current maintainers:
 * Valentin Guignon (guignonv) - https://www.drupal.org/u/guignonv
