<?php

/**
 * @file
 * BrAPI site entity API.
 */

/**
 * Determines whether the given user has access to a BrAPI site.
 *
 * @param string $op
 *   The operation being performed. One of 'view', 'edit' or 'create'.
 * @param mixed $brapi_site
 *   Optionally a BrAPI site to check access for. Currently ignored: access for
 *   all BrAPI sites is determined.
 * @param object $account
 *   The user to check for. Leave it to NULL to check for the global user.
 *
 * @return bool
 *   Whether access is allowed or not.
 */
function brapi_site_access($op, $brapi_site = NULL, $account = NULL) {
  if (user_access('administer brapi sites', $account)
      || (('view' == $op) && user_access('view any brapi site entity', $account))
      || (('edit' == $op) && user_access('edit any brapi site entity', $account))
      || (('create' == $op) && user_access('create brapi site entities', $account))) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Fetch a BrAPI site entity.
 *
 * @param int $bsid
 *   Integer specifying the BrAPI site id.
 * @param bool $reset
 *   A boolean indicating that the internal cache should be reset.
 *
 * @return BrapiSite
 *   A fully-loaded $brapi_site entity or FALSE if it cannot be loaded.
 *
 * @see brapi_site_load_multiple()
 */
function brapi_site_load($bsid, $reset = FALSE) {
  $brapi_sites = brapi_site_load_multiple(array($bsid), array(), $reset);
  return reset($brapi_sites);
}

/**
 * Load multiple BrAPI sites based on certain conditions.
 *
 * @param int $bsids
 *   An array of BrAPI site IDs.
 * @param array $conditions
 *   An array of conditions to match against the {brapi_site} table.
 * @param bool $reset
 *   A boolean indicating that the internal cache should be reset.
 *
 * @return array
 *   An array of BrAPI site objects, indexed by bsid.
 *
 * @see entity_load()
 * @see brapi_site_load()
 */
function brapi_site_load_multiple($bsids = array(), $conditions = array(), $reset = FALSE) {
  return entity_load('brapi_site', $bsids, $conditions, $reset);
}

/**
 * Deletes a BrAPI site.
 */
function brapi_site_delete(BrapiSite $brapi_site) {
  $brapi_site->delete();
}

/**
 * Delete multiple brapi_sites.
 *
 * @param array $bsids
 *   An array of BrAPI site IDs.
 */
function brapi_site_delete_multiple(array $bsids) {
  entity_get_controller('brapi_site')->delete($bsids);
}

/**
 * Create a BrAPI site object.
 */
function brapi_site_create($values = array()) {
  return entity_get_controller('brapi_site')->create($values);
}

/**
 * Saves a BrAPI site to the database.
 *
 * @param BrapiSite $brapi_site
 *   The brapi_site object.
 */
function brapi_site_save(BrapiSite $brapi_site) {
  return $brapi_site->save();
}

/**
 * URI callback for BrAPI sites.
 *
 * @param BrapiSite $brapi_site
 *   The brapi_site object.
 */
function brapi_site_uri(BrapiSite $brapi_site) {
  return array(
    'path' => 'brapi_site/' . $brapi_site->bsid,
  );
}

/**
 * Menu title callback for showing individual entities.
 *
 * @param BrapiSite $brapi_site
 *   The brapi_site object.
 */
function brapi_site_page_title(BrapiSite $brapi_site) {
  return $brapi_site->title;
}

/**
 * Sets up content to show an individual BrAPI site.
 *
 * @param BrapiSite $brapi_site
 *   The brapi_site object.
 * @param string $view_mode
 *   View mode.
 */
function brapi_site_page_view(BrapiSite $brapi_site, $view_mode = 'full') {
  $controller = entity_get_controller('brapi_site');
  $content = $controller->view(array($brapi_site->bsid => $brapi_site));
  drupal_set_title($brapi_site->title);
  return $content;
}

/**
 * Encrypt a given clear password using BrAPI salt.
 */
function brapi_site_encrypt_password($password) {
  // Get current salt.
  $password_salt = variable_get('brapi_salt');
  if (!$password_salt) {
    drupal_set_message(t('Failed to get BrAPI password salt! You may have to uninstall and reinstall this module.'), 'error');
    return '';
  }

  // Get password length.
  $password_length = strlen($password);
  // Make sure salt length is enough.
  $full_password_salt = $password_salt;
  while (strlen($full_password_salt) < $password_length) {
    $full_password_salt .= $password_salt;
  }

  // Encrypt.
  $crypted_password = '';
  for ($i = 0; $i < $password_length; ++$i) {
    $crypted_byte = chr(ord($password[$i]) ^ ord($full_password_salt[$i]));
    $crypted_password .= $crypted_byte;
  }

  return base64_encode($crypted_password);
}

/**
 * Decrypt a given crypted password using BrAPI salt.
 */
function brapi_site_decrypt_password($crypted_password) {
  // Get current salt.
  $password_salt = variable_get('brapi_salt');
  if (!$password_salt) {
    drupal_set_message(t('Failed to get BrAPI password salt! You may have to uninstall and reinstall this module.'), 'error');
    return '';
  }

  $crypted_password = base64_decode($crypted_password);

  // Get password length.
  $password_length = strlen($crypted_password);
  // Make sure salt length is enough.
  $full_password_salt = $password_salt;
  while (strlen($full_password_salt) < $password_length) {
    $full_password_salt .= $password_salt;
  }

  // Decrypt.
  $password = '';
  for ($i = 0; $i < $password_length; ++$i) {
    $clear_byte = chr(ord($crypted_password[$i]) ^ ord($full_password_salt[$i]));
    $password .= $clear_byte;
  }

  return $password;
}
