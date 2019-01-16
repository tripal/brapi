
How to Implement Calls
=======================

For each call you would like to implement:

 1. Create a function in ``api/brapi.calls.inc`` before ``brapi_v1_external_call_json()`` function and ideally near related functions with a name like: ``brapi_v1_[call_name]_[call_version]_json`` (replace non-word characters in call name by underscores and lowercase all the name).  For calls that may receive IDs, add a parameter with a NULL default value.  for example, ``brapi_v1_germplasm_germplasmdbid_13_json($germplasm_id = NULL)`` for BrAPI call ``germplasm/{germplasmDbId}`` following BrAPI v1.3 specifications. The function will return an ordered PHP array ``array($result, $metadata, $debug)`` that will reflect the JSON array returned by BrAPI.
 2. Update brapi_get_calls() in ``api/brapi.const.inc`` in order to make the ``callback versions`` subkeys of the call you are implementing to point to your new function.
 3. Then do one of the following depending on the call:

     - Call on a resource (ie. a resource ID may be provided)
     - Search call
     - Other type of call
     - Override existing implementation

Call on a resource
--------------------

In the ``brapi_get_data_mapping()`` function (in ``api/brapi.const.inc``), find the ``$brapi_data_mapping`` associative array and make sure the resource exists in that array (as first level key). For instance the ``marker`` key will hold data mapping for markers.

Fill the corresponding resource mapping if it is not completed yet. All is documented in the ``brapi_get_data_mapping()`` function code documentation.
The ``fields`` key holds the definition of both the BrAPI resource fields and internal fields. Internal fields might be used to compute BrAPI resource fields. Each field can be defined as one of the following:

 - a column of the Chado table associated with the resource,
 - a column of another Chado table linked to the table associated with the resource,
 - a function that will handle the field value and operations on it (create, read, update, delete or a subset of those operations)
 - an alias for another field.

Some BrAPI field values may be stored in several ways in Chado, depending how people use Chado. Such fields should always be defined as alias and the various ways of storing the valus should use internal field names. 

For instance, germplasm 'species' field could be stored as a stockprop (plain text), a stock_cvterm (in a CV holding species names), in the organism table (as species name), in the phylonode table (when the linked organism correspond to a sub-taxa rather than the species) or maybe in another creative way. Therefore, for each of those case, an internal field is created (let’s say 'speciesProp', 'speciesCVTerm', 'speciesOrganism', 'speciesPhylonode' and 'speciesMyWay') and the main field 'species' used by BrAPI will be an alias to one of those internal fields. That alias can be modified by the administrator to make it point to the right internal field on the BrAPI administration interface. The alias default value will be then overridden by the admin settings right after the definition of the ``$brapi_data_mapping`` array.

Also note that deprecated field names, for instance 'germplasmSpecies' may also be set as aliases to the non-deprecated fields replacing them (here 'species') that might be an alias as well (ie. alias > alias > internal field).

The implementation of your ``brapi_v1_[call_name]_[call_version]_json($resource_id)`` that will handle CRUD operations will look like this:

.. code-block:: php

   function brapi_v1_[call_name]_[call_version]_json($resource_id)  {
    $actions = array( 
      'create' => 'brapi_v1_create_[call_name]_[call_version]_json', 
      'read'   => 'brapi_v1_read_[call_name]_[call_version]_json', 
      'update' => 'brapi_v1_update_[call_name]_[call_version]_json', 
      'delete' => 'brapi_v1_delete_[call_name]_[call_version]_json', 
      'list'   => 'brapi_v1_search_[call_name]_[call_version]_json',
    ); 
    return brapi_process_crud('[call_name]', $actions, '[resource_type]', $resource_id);
  }

 .. note::
  You may only need to implement a subset of CRUD. In this case just remove unimplemented operations from the $actions array.

Then you will have to define each function that will handle a CRUD operation. The following provides an example of a read operation:

.. code-block:: php

  function brapi_v1_read_[call_name]_[call_version]_json($resource) { 
    $metadata = brapi_prepare_metadata(1); // We return 1 element. 
    $debug_data = array(); 

    // We delegate data loading to a specialized function. 
    $resource_type_data =     brapi_get_[resource_type]_[call_version]_details($resource);
    $data = array('result' => $resource_type_data); 

    brapi_aggregate_call($data, $metadata, $debug_data); 
    return array( 
      $data, 
      $metadata, 
      $debug_data, 
    );
  }

The function returns an array of 3 elements used in the BrAPI JSON answer. 

Search call
-------------

As for the call on a resource, the data mapping must be defined.
The search function should look like this: 

.. code-block:: php

  function brapi_v1_search_[call_name]_[call_version]_json() { 
    $cv_settings = brapi_get_cv_settings(); 

    $parameters = array( 
      'selectors' => array(
        [chado_filter] 
      ), 
      // We provide the function that will load the resource BrAPI fields.     'get_object_details' => 'brapi_get_[resource_type]_[call_version]_details', 
    ); 

    // This function will magically do the searching, handle pagination
    // and user filters for you. 
    return brapi_v1_object_search_json('[resource_type]', $parameters);
   }

The ``selectors`` key is used to set a default filter on which other filters will be later added. We use the same format for ``[chado_filter]`` as is required by ``chado_generate_vars``. For instance, if your resource is germplas, you will want to return only germplasm from the Chado stock table. This requires filtering by stock.type_id to ensure only stocks with a type specified in the configuration are returned. Thus, in the code snippet above [chado_filter] would be ``type_id => [cvterm_ids entered in config form]``.

Both resource create, read, update and list/search operations use the following specialized function:

.. code-block:: php

   function brapi_get_[resource_type]_[call_version]_details($resource) { 
      $resource_type_data = array(); 

      $fields = array( 
       [list_of_brapi_field_names_to_output], 
      ); 

      // Some additional fields may be requested in the query;
      // auto-add them to the list. 
      $fields = brapi_merge_fields_to_include($fields); 

      // Load/fetch each requested field value. 
      foreach ($fields as $field_name) { 
        // All is magically handled by the data mapping defined earlier.     $resource_type_data[$field_name] =       brapi_get_field('[resource_type]', $resource, $field_name); 
      } 

      return $resource_type_data; 
  }


Other type of call
--------------------

Any BrAPI call function should do something similar to: 

.. code-block:: php

  function brapi_[call_name]_[call_version]_json() { 
    $debug = array(); 
    $metadata = brapi_prepare_metadata([element_count_or_1_or_nothing]); 
    $data = array(); 

    // compute result data. 

    // Optional: aggregate data from external calls.   brapi_aggregate_call($data, $metadata, $debug); 

    // Return the BrAPI array in PHP that will be transcribed into a JSON string. 
    return array(
      array('result' => array('data' => $data)), 
      $metadata, 
      $debug, 
    );
   }


Override existing implementation
----------------------------------

Existing implementations car be overridden from another Drupal module using the following hooks:

 - ``hook_brapi_calls_alter(&brapi_calls)`` to replace the functions used by calls. Use this method of you don't need to use existing call functions or if you need to implement your own handler for the way you store your data.

 - ``hook_brapi_CALL_FUNC_NAME_alter(&$data, &$context)`` to keep existing call function (and setting selection) but just alter the default output result. This can be useful if you just want to remove (access restriction) or add data.
