
Configuration
===============

Configure Tripal-BrAPI in "Administration > Tripal > Extensions > Breeding API > Settings" (path /admin/tripal/extension/brapi/configuration). The settings are organized by sections:

Example value settings
-----------------------

You can specify here which identifiers should be used to demo the calls on the overview page (path brapi/overview). The identifiers correspond to the names inside curly brackets used in call URLs. These settings are optional and do not impact BrAPI behavior.

Storage options
-----------------

This is the place where you specify how you store your data in Chado and how BrAPI can find it. For instance, if you don't store the common crop name of your stock in the stockprop table, BrAPI can use the organism table instead. In this case, you will change the "Common crop name storage" settings from "Stored in stockprop table" to "Stored in organism table". Then Tripal-BrAPI module will find the common crop name using stock.organism_id --> organism table --> organism.common_name. However, if you do use the stockprop table, then you must ensure you also setup the appropriate cvterm_id for "commonCropName" in the MCPD settings.

All the other parameters have a similar approaches. The "Date storage format" specifies the way your dates are stored and not the way BrAPI will display them (which is in the specifications). As dates are stored as strings in the value field of property, they can be stored in a human-readable manner as well as in timestamp format.

Controlled vocabulary settings
--------------------------------

This is where you associate BrAPI terms used by BrAPI calls to corresponding CV terms available in Chado and used by the corresponding field ([table]prop.type_id or [table]_cvterm.cvterm_id). Use the auto-completion feature to find the corresponding terms from the appropriate Controlled Vocabulary (CV). Make sure the numeric value of the identifier of each cvterm, ``(cvterm_id)``, is present; the term name can be omitted. In some cases, you may specify several CV terms by separating them with comma.

MCPD settings
---------------

BrAPI uses MCPD (Multi-Crop Passport Descriptor controlled vocabulary). If you use your own terms rather than the MCPD ones, you can override these settings here the same way you did for "Controlled vocabulary settings".

Germplasm attribute settings
-----------------------------

These settings hold attribute categories and germplasm attributes stored in your Chado database. Category names are case sensitive: make sure you always use the same case. For each attribute you define, you can specify either the Chado Controlled Vocabulary (cv_id) that holds all the terms used by the attribute or a list of Controlled Vocabulary terms (cvterm_id) used. The BrAPI system will look for attributes either in the stockprop table if they have a type_id corresponding to the selected Controlled Vocabulary terms and in stock_cvterm table if they have a corresponding cvterm_id. For stockprop, the value field will be returned as the attribute value while for the stock_cvterm table, the Controlled Vocabulary term name (cvterm.name) will be returned as the value. Only the attributes that you define here will be exposed by BrAPI.

.. warning::

  Category names are case sensitive: make sure you always use the same case.

Call aggregation options
--------------------------

This is where you can setup call aggregation or proxy-ing.

 - "Call aggregation" is a way to complete missing fields (null) returned by a given local BrAPI call with field values provided by the same call run on other BrAPI instances.
 - Proxy-ing is relaying a BrAPI call to another BrAPI instance and serve its result to the client without having him/her to know about the other BrAPI instance: the client only sees one BrAPI en point which make his/her life easier.

With this Tripal-BrAPI module, you can select which call you wish to aggregate with which other BrAPI instances. If you don't provide any data for a given call which is aggregated, all the field values from the other instance will be used, which is basically "proxy-ing" the call. In order to use aggregation, you need first to record external BrAPI sites in the system. To do so, you will have to go to the "Content > BrAPI Site References" page (path admin/content/brapi_site) and add new references. Then you can go back to the BrAPI setting page and will be able to select one of your referenced BrAPI sites for the call you would like to aggregate or proxy.

Permissions
-------------

Configure user permissions in "Administration > People > Permissions" (/admin/people/permissions):

  - "Use Breeding API": allows users to access to the Breeding API. Roles having this permission can not alter data but have read access to all the data available through the Breeding API.

  - "Update through Breeding API": allows users to modify database content. Roles with this permission can add new entries and update or remove exiting ones.

  - "Administer Breeding API": allows users to change the Breeding API settings such as the CV term uses and the default entries to use as example.
