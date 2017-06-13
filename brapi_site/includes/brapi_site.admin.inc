<?php

/**
 * @file
 * BrAPI site administration functions.
 */

/**
 * Form callback wrapper: create or edit a BrAPI site.
 *
 * @param BrapiSite $brapi_site
 *   The BrAPI site object being edited by this form.
 *
 * @see brapi_site_edit_form()
 */
function brapi_site_form_wrapper(BrapiSite $brapi_site) {
  // Add the breadcrumb for the form's location.
  brapi_site_set_breadcrumb();
  return drupal_get_form('brapi_site_edit_form', $brapi_site);
}

/**
 * Form callback wrapper: delete a BrAPI site.
 *
 * @param BrapiSite $brapi_site
 *   The brapi_site object being edited by this form.
 *
 * @see brapi_site_edit_form()
 */
function brapi_site_delete_form_wrapper(BrapiSite $brapi_site) {
  // Add the breadcrumb for the form's location.
  brapi_site_set_breadcrumb();
  return drupal_get_form('brapi_site_delete_form', $brapi_site);
}

/**
 * Form callback: create or edit a BrAPI site.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 * @param BrapiSite $brapi_site
 *   The BrAPI site object to edit or for a create form an empty BrAPI site
 *   object with only a BrAPI site type defined.
 */
function brapi_site_edit_form($form, &$form_state, BrapiSite $brapi_site) {
  // Add the field related form elements.
  $form_state['brapi_site'] = $brapi_site;

  // Machine name stuff.
  $machine_name_pattern = '[^0-9a-zA-Z-]+';
  $machine_name_replacement = '-';

  $form = array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('BrAPI site name'),
      '#description'   => t('Enter the name of the BrAPI site that will be displayed on user interface.'),
      '#default_value' => isset($brapi_site->title) ? $brapi_site->title : '',
      '#maxlength' => 255,
      '#required' => TRUE,
      '#weight' => -2,
      '#field_suffix'  => '<br/>',
    ),
    'machine_name' => array(
      '#type' => 'machine_name',
      '#default_value' => isset($brapi_site->machine_name) ? $brapi_site->machine_name : '',
      '#maxlength' => 16,
      '#machine_name' => array(
        'exists' => 'brapi_site_name_exists',
        'source' => array('title'),
        'label' => t('BrAPI site reference machine name (unique identifier)'),
        'replace_pattern' => $machine_name_pattern,
        'replace' => $machine_name_replacement,
      ),
      '#required'         => TRUE,
      '#element_validate' => array('brapi_site_machine_name_validate'),
      '#weight'           => -1,
    ),
    'url' => array(
      '#type' => 'textfield',
      '#title' => t('BrAPI site service URL'),
      '#description' => t('The URL should begin with &quot;http://&quot; or &quot;https://&quot; and end with &quot;/brapi/v1&quot;.'),
      '#required' => TRUE,
      '#default_value' => isset($brapi_site->url) ? $brapi_site->url : '',
      '#element_validate' => array('brapi_site_url_validate'),
      '#weight' => 0,
    ),
    'version' => array(
      '#type'          => 'select',
      '#title'         => t('BrAPI service version'),
      '#description'   => t('The (compatible) version of the BrAPI implementation.'),
      '#required'      => TRUE,
      '#options' => array(
        1 => t('version 1'),
      ),
      '#default_value' => isset($brapi_site->version) ? $brapi_site->version : 1,
      '#weight'        => 1,
    ),
    'username' => array(
      '#type'          => 'textfield',
      '#title'         => t('User login'),
      '#description'   => t('User login used to access to the BrAPI service.'),
      '#required'      => FALSE,
      '#default_value' => (
        isset($brapi_site->username) ? $brapi_site->username : ''
      ),
      '#weight'        => 2,
    ),
    'password' => array(
      '#type'          => 'password_confirm',
      '#title'         => t('BrAPI service password (optionnaly associated with the user login above). Leave empty to clear password.'),
      '#required'      => FALSE,
      '#weight'        => 3,
    ),
    'description' => array(
      '#type'          => 'textarea',
      '#title'         => t('BrAPI site description'),
      '#description'   => t('Contains the description of the referenced BrAPI site.'),
      '#required'      => FALSE,
      '#default_value' => isset($brapi_site->description) ? $brapi_site->description : '',
      '#weight'        => 4,
    ),
  );

  // This is necessary for field_default_form() not to yell at us.
  $form['#parents'] = array();

  field_attach_form('brapi_site', $brapi_site, $form, $form_state);

  $form['actions'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('form-actions')),
    '#weight' => 400,
  );

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save BrAPI site'),
    '#submit' => $submit + array('brapi_site_edit_form_submit'),
  );

  if (!empty($brapi_site->bsid)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete BrAPI site reference'),
      '#suffix' => l(t('Cancel'), 'admin/content/brapi_site'),
      '#submit' => $submit + array('brapi_site_form_submit_delete'),
      '#weight' => 45,
    );
  }

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'brapi_site_edit_form_validate';
  return $form;
}

/**
 * Callback to check if a BrAPI site machine name is already in use.
 */
function brapi_site_name_exists($value) {
  $name_exists = db_query_range(
    'SELECT 1 FROM {brapi_site} WHERE machine_name = :name',
    0,
    1,
    array(':name' => $value)
  )->fetchField();

  return $name_exists;
}

/**
 * BrAPI site machine_name validation handler.
 *
 * @param array $element
 *   a Drupal form element array.
 * @param array $form_state
 *   a Drupal form state array.
 * @param array $form
 *   a Drupal form array.
 */
function brapi_site_machine_name_validate($element, &$form_state, $form) {
  if (empty($element['#value'])) {
    form_error($element, t('The machine name field is required.'));
  }
  elseif (16 < strlen($element['#value'])) {
    form_error($element, t('The machine name field must be non longer than 16 characters.'));
  }
}

/**
 * BrAPI site URL validation handler.
 *
 * @param array $element
 *   a Drupal form element array.
 * @param array $form_state
 *   a Drupal form state array.
 * @param array $form
 *   a Drupal form array.
 */
function brapi_site_url_validate($element, &$form_state, $form) {
  // Check URL.
  if (!valid_url($element['#value'], TRUE)) {
    form_error($element, t("Invalid URL."));
  }
}

/**
 * Form API validate callback for the BrAPI site form.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 */
function brapi_site_edit_form_validate(&$form, &$form_state) {
  $brapi_site = $form_state['brapi_site'];

  // Notify field widgets to validate their data.
  field_attach_form_validate('brapi_site', $brapi_site, $form, $form_state);
}

/**
 * Form API submit callback for the BrAPI site form.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 */
function brapi_site_edit_form_submit(&$form, &$form_state) {

  $brapi_site = entity_ui_controller('brapi_site')->entityFormSubmitBuildEntity($form, $form_state);

  // Save the BrAPI site and go back to the list of BrAPI sites.
  $brapi_site->save();
  $form_state['redirect'] = 'admin/content/brapi_site';
}

/**
 * Form API submit callback for the delete button.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 */
function brapi_site_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'brapi_site/' . $form_state['brapi_site']->bsid . '/delete';
}

/**
 * Form callback: confirmation form for deleting a BrAPI site.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 * @param BrapiSite $brapi_site
 *   The BrAPI site to delete.
 *
 * @see confirm_form()
 */
function brapi_site_delete_form($form, &$form_state, BrapiSite $brapi_site) {
  $form_state['brapi_site'] = $brapi_site;

  $form['#submit'][] = 'brapi_site_delete_form_submit';

  $form = confirm_form($form,
    t('Are you sure you want to delete BrAPI site %name?', array('%name' => $brapi_site->title)),
    'admin/content/brapi_site',
    '<p>' . t('This action cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}

/**
 * Submit callback for brapi_site_delete_form.
 *
 * @param array $form
 *   a Drupal form array.
 * @param array $form_state
 *   a Drupal form state array.
 */
function brapi_site_delete_form_submit($form, &$form_state) {
  $brapi_site = $form_state['brapi_site'];
  brapi_site_delete($brapi_site);

  drupal_set_message(t('The BrAPI site %name has been deleted.', array('%name' => $brapi_site->title)));
  watchdog('brapi_site', 'Deleted BrAPI site %name.', array('%name' => $brapi_site->title));

  $form_state['redirect'] = 'admin/content/brapi_site';
}

/**
 * Page to add BrAPI site Entities.
 */
function brapi_site_add_page() {
  $controller = entity_ui_controller('brapi_site');
  return $controller->addPage();
}

/**
 * Sets the breadcrumb for administrative BrAPI site pages.
 */
function brapi_site_set_breadcrumb() {
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Content'), 'admin/content'),
    l(t('BrAPI site references'), 'admin/content/brapi_site'),
  );

  drupal_set_breadcrumb($breadcrumb);
}
