<?php

namespace Drupal\Tests\brapi\Unit;

include "../modules/contrib/brapi/brapi.module";

use Drupal\Tests\UnitTestCase;
use Drupal\brapi\Entity\BrapiDatatype;

/**
 * The class to test BrapiDatatype.
 *
 * @group brapi
 */
class BrapiDatatypeTest extends UnitTestCase {

  /**
   * Data provider for testBrapiDatatype().
   */
  public function provideTestBrapiDatatype() {
    return [
      [
        'v2-2.1-ListDetails',
        [
          "uuid" => "123456789",
          "langcode" => "en",
          "status" => true,
          "dependencies" => [],
          "id" => "v2-2.1-ListDetails",
          "label" => "List for BrAPI v2.1",
          "contentType" => "brapi_list:brapi_list",
          "contentFieldPath" => null,
          "mapping" => [
            "additionalInfo" => [
              "field" => "additional_info",
              "custom" => "",
              "is_json" => false,
            ],
            "data" => [
              "field" => "data",
              "custom" => "$.data[*].value",
              "is_json" => true,
            ],
            "dateCreated" => [
              "field" => "created",
              "custom" => "",
              "is_json" => false,
            ],
            "dateModified" => [
              "field" => "changed",
              "custom" => "",
              "is_json" => false,
            ],
            "externalReferences" => [
              "field" => "external_references",
              "custom" => "",
              "is_json" => false,
            ],
            "listDbId" => [
              "field" => "id",
              "custom" => "",
              "is_json" => false,
            ],
            "listDescription" => [
              "field" => "list_description",
              "custom" => "",
              "is_json" => false,
            ],
            "listName" => [
              "field" => "list_name",
              "custom" => "",
              "is_json" => false,
            ],
            "listOwnerName" => [
              "field" => "_custom",
              "custom" => "$.user_id[0].value.name[0].value",
              "is_json" => false,
            ],
            "listOwnerPersonDbId" => [
              "field" => "_custom",
              "custom" => "$.user_id[0].target_id",
              "is_json" => false,
            ],
            "listSize" => [
              "field" => "list_size",
              "custom" => "",
              "is_json" => false,
            ],
            "listSource" => [
              "field" => "list_source",
              "custom" => "",
              "is_json" => false,
            ],
            "listType" => [
              "field" => "list_type",
              "custom" => "",
              "is_json" => false,
            ],
          ],
        ],
      ],
    ];
  }

  /**
   * Test value extraction from raw data using given mappings.
   *
   * @dataProvider provideTestBrapiDatatype
   */
  public function testBrapiDatatype($test_datatype_name, $test_datatype_config) {
    $brapi_datatype = new BrapiDatatype($test_datatype_config, $test_datatype_name);
    $version = $brapi_datatype->getBrapiVersion();
    $this->assertEquals('v2', $version, 'Failed with ');
  }

}
