
Hook Overview
===============

-  ``hook_brapi_cv_settings_alter(&settings)``: allows other module to alter CV settings. See also ``brapi_get_cv_settings()`` documentation in ``api/brapi.api.inc``.

-  ``hook_brapi_data_mapping_alter(&$brapi_data_mapping)``: allows other module to alter BrAPI data mapping settings. See also ``brapi_get_data_mapping()`` documentation in ``api/brapi.const.inc``.

-  ``hook_brapi_object_selector_alter(&$selector, $context)``: allows other module to alter Chado object loaded by BrAPI when a DbId has been specified for a given call.

-  ``hook_brapi_calls_alter(&brapi_calls)``: allows other module to alter BrAPI call settings. While you can replace call settings and especially the callback function, you should better use ``hook_brapi_CALL_FUNC_NAME_alter`` hook to replace BrAPI calls in order to allow other modules to alter the call answer if they need to as well. This hook should be used to change call version settings or supported methods or datatypes or aggregation option only. See also ``brapi_get_calls()`` documentation in ``api/brapi.const.inc``.

-  ``hook_brapi_CALL_FUNC_NAME_alter(&$data, &$context)``: allows other module to alter the result of a BrAPI call. ``$data`` contains the result structure currently returned by the call and ``$context`` contains the metadata and debug strings. See also ``brapi_call_wrapper()`` documentation in ``api/brapi.calls.inc``.

-  ``hook_brapi_CALL_FUNC_NAME_brapi_error_alter(&$output)``: allows other module to alter error raised by BrAPI (BrAPI exceptions associated to specific HTTP error codes (like 400 bad request, 404 not found, 501 not implemented and such). It is also possible to replace errors by results since $output contains the full JSON structure returned by BrAPI. This may be useful for unimplemented calls raising 501 errors or values not found by current implementation raising 404 if your module can handle those. See also ``brapi_call_wrapper()`` documentation in ``api/brapi.calls.inc``.

-  ``hook_brapi_CALL_FUNC_NAME_error_alter()``: allows other module to alter other type of errors (not raised by BrAPI, typically PHP exceptions). See also ``brapi_call_wrapper()`` documentation in ``api/brapi.calls.inc``.
