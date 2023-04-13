<?php

namespace Drupal\brapi\Form;

use Drupal\brapi\Entity\BrapiList;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;

/**
 * A simple form that displays a select box and submit button.
 *
 * This form will be be themed by the 'theming_example_select_form' theme
 * handler.
 */
class BrapiAdminForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'brapi_admin_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    // Get current settings.
    $config = \Drupal::config('brapi.settings');

    // Versions.
    $form['versions'] = [
      '#type'  => 'details',
      '#title' => $this->t('Version Management'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];

    // Get BrAPI versions.
    $versions = brapi_available_versions();
    
    foreach ($versions as $version => $version_definition) {
      $version_options = array_keys($version_definition);
      $version_options = array_combine($version_options, $version_options);
      if (!empty($version_options)) {
        $form['versions'][$version] = [
          '#type' => 'checkbox',
          '#title' => $this->t(
            'Enable BrAPI %version endpoint',
            ['%version' => $version]
          ),
          '#return_value' => TRUE,
          '#default_value' => $config->get($version),
        ];
        $form['versions'][$version . '_definition'] = [
          '#type' => 'select',
          '#options' => $version_options,
          '#title' => $this->t(
            'Select an implementation for %version',
            ['%version' => $version]
          ),
          '#default_value' => $config->get($version . 'def'),
          '#wrapper_attributes' => [
              'class' => ['container-inline'],
          ],
          '#states' => [
            'visible' => [
              ':input[name="' . $version . '"]' => ['checked' => TRUE],
            ],
          ],
        ];
      }
    }
    // @todo Add existing/obsolete mapping management.
    $form['mappings'] = [
      '#type'  => 'details',
      '#title' => $this->t('Mappings'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];
    $form['mappings']['mapping_list'] = [
      '#type'  => 'markup',
      '#markup'  => 'TODO: display the list of defined mappings by sub-versions (with counts), highlight disabled ones and add button to remove all their related mappings.<br/>',
    ];
    $form['mappings']['manage_link'] = [
      '#title' => $this->t('Manage all mappings'),
      '#type' => 'link',
      '#url' => \Drupal\Core\Url::fromRoute('entity.brapidatatype.list'),
    ];

    // Paging group.
    $form['paging'] = [
      '#type'  => 'details',
      '#title' => $this->t('Paging'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];

    // Default page size.
    $default_page_size = $config->get('page_size') ?: BRAPI_DEFAULT_PAGE_SIZE;
    $form['paging']['page_size'] = [
      '#type' => 'number',
      '#title' => $this->t('Default number of result in pages'),
      '#default_value' => $default_page_size,
      '#min' => 1,
      '#required' => TRUE,
    ];

    // Maximum allowed Pagesize value.
    $page_size_max =
      $config->get('page_size_max')
      ?? max(BRAPI_DEFAULT_PAGE_SIZE_MAX, $default_page_size)
    ;
    $form['paging']['page_size_max'] = [
      '#type' => 'number',
      '#title' => $this->t('Maximum number of results allowed per pages'),
      '#default_value' => $page_size_max,
      '#description' => $this->t('As BrAPI applications and users may specify very high values for the pageSize parameter, this setting prevents abuses by limiting the allowed number of values per page.'),
      '#min' => 1,
      '#required' => FALSE,
    ];

    // Access restriction group.
    $form['access'] = [
      '#type'  => 'details',
      '#title' => $this->t('Access Restrictions'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];

    // Token.
    $form['access']['token_default_lifetime'] = [
      '#type' => 'number',
      '#title' => $this->t('Default token lifetime'),
      '#description' => $this->t('Maximum lifetime in seconds.'),
      '#default_value' => $config->get('token_default_lifetime') ?? BRAPI_DEFAULT_TOKEN_LIFETIME,
      '#min' => 1,
      '#required' => TRUE,
    ];

    $form['access']['insecure'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Allow the use of token over insecure connections (ie. not HTTPS)'),
      '#return_value' => TRUE,
      '#default_value' => $config->get('insecure') ?? FALSE,
    ];


    // Search group.
    $form['search'] = [
      '#type'  => 'details',
      '#title' => $this->t('Search'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];
    $form['search']['search_default_lifetime'] = [
      '#type' => 'number',
      '#title' => $this->t('Default search lifetime'),
      '#description' => $this->t('Minimum lifetime in seconds.'),
      '#default_value' => $config->get('search_default_lifetime') ?? BRAPI_DEFAULT_SEARCH_LIFETIME,
      '#min' => 1,
      '#required' => TRUE,
    ];

    // Clear search cache button.
    $form['search']['clear_cache'] = [
      '#type' => 'submit',
      '#value' => $this->t('Clear search cache'),
      '#submit' => ['::clearSearchCache'],
    ];

    // Server info.
    $sys_config = \Drupal::config('system.site');
    $form['server_info'] = [
      '#type'  => 'details',
      '#title' => $this->t('Server Info'),
      '#open'  => TRUE,
      '#tree'  => FALSE,
    ];
    $form['server_info']['server_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Server name'),
      '#default_value' => $config->get('server_name') ?? $sys_config->get('name'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['server_description'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Server description'),
      '#default_value' => $config->get('server_name') ?? $sys_config->get('slogan'),
      '#size' => 60,
    ];
    $form['server_info']['contact_email'] = [
      '#type' => 'email',
      '#title' => $this->t('Contact e-mail'),
      '#default_value' => $config->get('contact_email') ?? $sys_config->get('mail'),
    ];
    $form['server_info']['documentation_url'] = [
      '#type' => 'url',
      '#title' => $this->t('Documentation URL'),
      '#default_value' =>
        $config->get('documentation_url')
        ?: Url::fromRoute('brapi.documentation', [], ['absolute' => TRUE])->toString()
      ,
    ];
    $form['server_info']['location'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Location'),
      '#default_value' => $config->get('location'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['organization_name'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Organisation name'),
      '#default_value' => $config->get('organization_name'),
      '#size' => 60,
      '#maxlength' => 128,
    ];
    $form['server_info']['organization_url'] = [
      '#type' => 'url',
      '#title' => $this->t('Organisation URL'),
      '#default_value' => $config->get('organization_url'),
    ];

    // Form save button.
    $form['submit'] = [
      '#type' => 'submit',
      '#value' => $this->t('Save'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $config = \Drupal::service('config.factory')->getEditable('brapi.settings');
    // Save version settings.
    $versions = brapi_available_versions();
    foreach ($versions as $version => $version_definition) {
      $definition = $form_state->getValue($version . '_definition') ?? '';
      if (!empty($version_definition[$definition])) {
        $config
          ->set($version, $form_state->getValue($version) ?? FALSE)
          ->set($version . 'def', $definition)
        ;
      }
    }

    // Check if BrAPI v2 is enabled and ensure a List mapping is there.
    if ($form_state->getValue('v2') ?? FALSE) {
      $mapping_sm = \Drupal::entityTypeManager()->getStorage('brapidatatype');
      // Check if a mapping for lists already exists.
      $active_def = $form_state->getValue('v2_definition') ?? '';
      foreach (['ListDetails', 'ListSummary'] as $list_datatype) {
        $list_datatype_id = brapi_generate_datatype_id($list_datatype, 'v2', $active_def);
        $brapi_list = $mapping_sm->load($list_datatype_id);
        if (!$brapi_list) {
          // No mapping. Create one.
          $brapi_definition = brapi_get_definition('v2', $active_def);
          $mapping = [];
          foreach (
            $brapi_definition["data_types"]["ListDetails"]["fields"]
            as $bfield => $fdef
          ) {
            // Change field name convention (CaMel to snake_case).
            // (?<!^) lookbehind to avoid a '_' at the begining of the name.
            $dfield = strtolower(preg_replace('/(?<!^)[A-Z]/', '_$0', $bfield));
            $mapping[$bfield] = [
              'field' => $dfield,
              'static' => '',
              // Use JSON for complex data and lists.
              'is_json' => (
                in_array($fdef['type'], ['string', 'integer', 'uuid'])
                ? FALSE
                : TRUE
              ),
              'subfield' => '',
            ];
          }

          $brapi_list = $mapping_sm->create([
            'id' => $list_datatype_id,
            'label' => "List for BrAPI v2.0",
            'contentType' => "brapi_list:brapi_list",
            'mapping' => $mapping,
          ]);
          $brapi_list->save();
        }
      }
    }

    // Save server settings.
    $config
      ->set('server_name', $form_state->getValue('server_name') ?? '')
      ->set('server_description', $form_state->getValue('server_description') ?? '')
      ->set('contact_email', $form_state->getValue('contact_email') ?? '')
      ->set('documentation_url', $form_state->getValue('documentation_url') ?? '')
      ->set('location', $form_state->getValue('location') ?? '')
      ->set('organization_name', $form_state->getValue('organization_name') ?? '')
      ->set('organization_url', $form_state->getValue('organization_url') ?? '')
      ->set('page_size', $form_state->getValue('page_size') ?? BRAPI_DEFAULT_PAGE_SIZE)
      ->set('page_size_max', $form_state->getValue('page_size_max') ?? BRAPI_DEFAULT_PAGE_SIZE_MAX)
      ->set('token_default_lifetime', $form_state->getValue('token_default_lifetime') ?? BRAPI_DEFAULT_TOKEN_LIFETIME)
      ->set('search_default_lifetime', $form_state->getValue('search_default_lifetime') ?? BRAPI_DEFAULT_SEARCH_LIFETIME)
      ->set('insecure', $form_state->getValue('insecure') ?? FALSE)
    ;
    $config->save();
    $this->messenger()->addMessage($this->t('BrAPI settings have been updated.'));
  }

  /**
   * {@inheritdoc}
   */
  public function clearSearchCache(array &$form, FormStateInterface $form_state) {
    // Save current config.
    $this->submitForm($form, $form_state);
    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('Search cache has been cleared.'));
  }

}
