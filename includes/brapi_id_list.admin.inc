<?php

/**
 * @file
 * BrAPI ID list administration functions.
 */

/**
 * Callback to check if a BrAPI site machine name is already in use.
 */
function brapi_id_list_name_exists($value) {
  $name_exists = db_query_range(
    'SELECT 1 FROM {brapi_id_list} WHERE machine_name = :name',
    0,
    1,
    array(':name' => $value)
  )->fetchField();

  return $name_exists;
}
