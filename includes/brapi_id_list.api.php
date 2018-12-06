<?php

/**
 * @file
 * BrAPI ID List entity API.
 */

/**
 * Fetch a BrAPI ID list entity.
 *
 * @param int $blid
 *   Integer specifying the BrAPI ID list id.
 * @param bool $reset
 *   A boolean indicating that the internal cache should be reset.
 *
 * @return BrapiIdList
 *   A fully-loaded $brapi_id_list entity or FALSE if it cannot be loaded.
 *
 * @see brapi_id_list_load_multiple()
 */
function brapi_id_list_load($blid, $reset = FALSE) {
  $brapi_id_lists = brapi_id_list_load_multiple(array($blid), array(), $reset);
  return reset($brapi_id_lists);
}

/**
 * Load multiple BrAPI ID lists based on certain conditions.
 *
 * @param int $blids
 *   An array of BrAPI ID list IDs.
 * @param array $conditions
 *   An array of conditions to match against the {brapi_id_list} table.
 * @param bool $reset
 *   A boolean indicating that the internal cache should be reset.
 *
 * @return array
 *   An array of BrAPI ID list objects, indexed by blid.
 *
 * @see entity_load()
 * @see brapi_id_list_load()
 */
function brapi_id_list_load_multiple($blids = array(), $conditions = array(), $reset = FALSE) {
  return entity_load('brapi_id_list', $blids, $conditions, $reset);
}

/**
 * Deletes a BrAPI ID list.
 */
function brapi_id_list_delete(BrapiIdList $brapi_id_list) {
  $brapi_id_list->delete();
}

/**
 * Delete multiple brapi_id_lists.
 *
 * @param array $blids
 *   An array of BrAPI ID list IDs.
 */
function brapi_id_list_delete_multiple(array $blids) {
  entity_get_controller('brapi_id_list')->delete($blids);
}

/**
 * Create a BrAPI ID list object.
 */
function brapi_id_list_create($values = array()) {
  return entity_get_controller('brapi_id_list')->create($values);
}

/**
 * Saves a BrAPI ID list to the database.
 *
 * @param BrapiIdList $brapi_id_list
 *   The brapi_id_list object.
 */
function brapi_id_list_save(BrapiIdList $brapi_id_list) {
  return $brapi_id_list->save();
}

/**
 * URI callback for BrAPI ID lists.
 *
 * @param BrapiIdList $brapi_id_list
 *   The brapi_id_list object.
 */
function brapi_id_list_uri(BrapiIdList $brapi_id_list) {
  return array(
    'path' => 'brapi_id_list/' . $brapi_id_list->blid,
  );
}
