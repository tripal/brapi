
Usage
======

Install the module, enable it and adjust the CV settings according to your Chado
database instance and the way you store your biological data. You may also setup
germplasm attribute settings, add some external BrAPI site references and setup
call aggregation if needed.

You can query the BrAPI service through the URL

.. code:: bash

  http(s)://<your-drupal-site>/brapi/v1/<service-name>?<query-parameters>

For example, ``https://www.crop-diversity.org/mgis/brapi/v1/calls``

It will return a JSON structure that can be processed by BrAPI-compliant
applications.

Testing your Instance
-----------------------

To test your instance, you can either use the RestClient plugin for your
favorite web browser client from http://restclient.net/ or use the provided
BrAPI query interface (path brapi/query). Note that when you use this interface
as admin, you will have an additional checkbox which can enable debug mode.

Javascript & dynamic HTML
---------------------------

If you want to add BrAPI fields on your pages that should be automatically and
dynamically populated using external BrAPI site, you can use the following HTML
snippet:

.. code:: html

  <form class="brapi-autoquery"
    action="https://BRAPI_SERVER/brapi/v1/SERVICE?PARAMETERS..." method="GET">
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

.. note::
  Array or object properties can not be used here.

The form can contain additional call parameters using hidden input or select
fields wrapped by an HTML element having the CSS class
"brapi-query-filter-post"
