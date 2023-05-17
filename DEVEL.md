Developper notes
================

Design
------
Each BrAPI data type mapping is defined by a \Drupal\brapi\Entity\BrapiDatatype
config entity. The entity holds the corresponding Drupal entity type name and
its field mapping and is used to extract BrAPI data from that data source.
So, biological data is stored in Drupal content entities and the BrAPI data is
extracted by BrAPI Data Type config entities by calling the method
->getBrapiData(). That method takes as parameter an associative array of BrAPI
field names with their associated values and it is used to filter the matching
content entities.


Code snippets
-------------

Get BrAPI definition for a given version:
> $brapi_20_definition = brapi_get_definition('v2', '2.0');
See brapi_get_definition() documentation in brapi.module for structure details.

Parse OpenAPI json:
> $f1 = file_get_contents('/path/to/brapi/versions/openapi_brapi-core_2.0.json');
> $def = brapi_open_api_to_definition([json_decode($f1, TRUE),]);
For multiple files:
> $def = brapi_open_api_to_definition([json_decode($f1, TRUE),json_decode($f2, TRUE),json_decode($f3, TRUE),json_decode($f4, TRUE),])
Save definition(s) to BrAPI Drupal module format:
> $json = json_encode($def, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
> $fdrupal = fopen('/path/to/brapi/definitions/brapi_v2_2.0.json', 'w');
> fwrite($fdrupal, $json);
> fclose($fdrupal);

Get BrAPI configuration:
> $config = \Drupal::config('brapi.settings');
Call settings:
> $call_settings = $config->get('calls');
Active BrAPI definition for v2:
$active_def = $config->get('v2def');

Get a BrAPI datatype mapping:
> $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');
> $germplasm_mapping = $mapping_loader->load('v2-2.0-GermplasmAttributeValue');

Extract BrAPI data from database:
-here for germplasm 123 and 456
> $brapi_data = $germplasm_mapping->getBrapiData(['germplasmDbId' => [123, 456,]]);

Pluralize a BrAPI term (like field names):
> $plural_term = brapi_get_term_plural($term);
Singularize a BrAPI term (like field names):
> $plural_term = brapi_get_term_singular($term);
