<?php

/**
 * @file
 * BrAPI site entity controller.
 */

/**
 * The controller for BrAPI site entities.
 */
class BrapiSiteController extends EntityAPIController {

  /**
   * Constructs a BrAPI site controller.
   */
  public function __construct($entityType) {
    parent::__construct($entityType);
  }

  /**
   * Creates a BrAPI site.
   *
   * @param array $values
   *   The fields of the BrAPI site.
   *
   * @return BrapiSite
   *   A BrAPI site object with all default fields initialized.
   */
  public function create(array $values = array()) {
    $values += array(
      'bsid' => NULL,
      'is_new' => TRUE,
      'title' => 'BrAPI Site',
      'machine_name' => 'brapi-site',
      'description' => '',
      'weight' => 0,
      'url' => '',
      'version' => 1,
      'username' => '',
      'password' => '',
    );

    $brapi_site = parent::create($values);
    return $brapi_site;
  }

  /**
   * Overriding the buildContent function to add entity specific fields.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {

    $content = parent::buildContent($entity, $view_mode, $langcode, $content);

    $content['brapi_site_url'] = array(
      '#markup' => theme(
        'brapi_site_url',
        array(
          'brapi_url' => check_plain($entity->url),
          'brapi_site' => $entity,
        )
      ),
    );

    if ('full' == $view_mode) {
      $content['brapi_site_version'] = array(
        '#markup' =>
        '<div class="brapi-site-version"><strong>BrAPI Version:</strong> <span>'
        . check_plain($entity->version)
        . '</span></div>',
      );

      $content['brapi_site_authentication'] = array(
        '#markup' => theme(
          'brapi_site_authentication',
          array(
            'brapi_site' => $entity,
          )
        ),
      );

      $content['brapi_site_description'] = array(
        '#markup' =>
        '<div class="brapi-site-description"><strong>Description:</strong> <p>'
        . check_plain($entity->description)
        . '</p></div>',
      );
    }

    return $content;
  }

  /**
   * Loads connections and decrypt passwords on the fly.
   */
  public function load($ids = FALSE, $conditions = array()) {
    $brapi_sites = parent::load($ids, $conditions);
    foreach ($brapi_sites as $brapi_site) {
      // Freshly loaded connection do not have 'password_decrypted' property
      // and therefor have a crypted password.
      if (!property_exists($brapi_site, 'password_decrypted')
          || !$brapi_site->password_decrypted) {
        $brapi_site->password = brapi_site_decrypt_password($brapi_site->password);
        $brapi_site->password_decrypted = TRUE;
      }
    }
    return $brapi_sites;
  }

  /**
   * Saves a connection data: saves crypted password.
   */
  public function save($brapi_site, DatabaseTransaction $transaction = NULL) {
    $return_value = NULL;

    // Appends a '/' to URL if needed.
    if (!preg_match('/\/$/', $brapi_site->url)) {
      $brapi_site->url .= '/';
    }

    // New connections do not have 'password_decrypted' property and if
    // existing ones do, check if password is crypted or not.
    if ((!empty($brapi_site->password))
        && ((!property_exists($brapi_site, 'password_decrypted')
          || $brapi_site->password_decrypted))) {
      // We got a clear password.
      $clear_password = $brapi_site->password;
      $brapi_site->password = brapi_site_encrypt_password($brapi_site->password);
      $return_value = parent::save($brapi_site, $transaction);
      // Put back decrypted password.
      $brapi_site->password = $clear_password;
    }
    else {
      $return_value = parent::save($brapi_site, $transaction);
    }
    return $return_value;
  }

}
