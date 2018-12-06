<?php

/**
 * @file
 * BrAPI ID list entity controller.
 */

/**
 * The controller for BrAPI ID list entities.
 */
class BrapiIdListController extends EntityAPIController {

  /**
   * Constructs a BrAPI ID list controller.
   */
  public function __construct($entityType) {
    parent::__construct($entityType);
  }

  /**
   * Creates a ID list.
   *
   * @param array $values
   *   The fields of the BrAPI ID list.
   *
   * @return BrapiIdList
   *   A BrAPI ID list object with all default fields initialized.
   */
  public function create(array $values = array()) {
    $values += array(
      'blid' => NULL,
      'is_new' => TRUE,
      'title' => 'My ID List',
      'description' => '',
      'list_owner' => '',
      'date_created' => '',
      'date_modified' => '',
      'list_type' => '',
      'list_source' => '',
      'data' => array(),
    );

    $brapi_id_list = parent::create($values);
    return $brapi_id_list;
  }

  /**
   * Overriding the buildContent function to add entity specific fields.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {

    $content = parent::buildContent($entity, $view_mode, $langcode, $content);

    $content['brapi_id_list'] = array(
      '#markup' => theme(
        'brapi_id_list',
        array(
          'brapi_id_list' => $entity,
        )
      ),
    );

    if ('full' == $view_mode) {
      $content['brapi_id_list_description'] = array(
        '#markup' =>
        '<div class="brapi-id-list-description"><strong>Description:</strong> <p>'
        . check_plain($entity->description)
        . '</p></div>',
      );
    }

    return $content;
  }

  /**
   * Loads ID list.
   */
  public function load($ids = FALSE, $conditions = array()) {
    $brapi_id_lists = parent::load($ids, $conditions);
    return $brapi_id_lists;
  }

  /**
   * Saves an ID list.
   */
  public function save($brapi_id_list, DatabaseTransaction $transaction = NULL) {
    return parent::save($brapi_id_list, $transaction);
  }

}
