<?php

namespace Drupal\brapi\Form;

use Drupal\brapi\Entity\BrapiDatatype;
use Drupal\Component\Serialization\Exception\InvalidDataTypeException;
use Drupal\Component\Serialization\Yaml;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\file\Entity\File;
use Symfony\Component\HttpFoundation\HeaderUtils;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\HttpFoundation\ResponseHeaderBag;

/**
 * A simple form that displays a select box and submit button.
 *
 * This form will be be themed by the 'theming_example_select_form' theme
 * handler.
 */
class BrapiDataTypesForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'brapi_datatypes_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    if ($form_state->has('confirm_delete') && !empty($form_state->get('confirm_delete'))) {
      return $this->buildDeleteConfirmForm($form, $form_state);
    }

    $form_state->set('page_num', 1);

    // V1 internal types.
    $v1_internal_types = [
      'WSMIMEDataTypes',
      'call',
      'successfulSearchResponse_result',
    ];
    // V2 internal types.
    $v2_internal_types = [
      'WSMIMEDataTypes',
      'ContentTypes',
      'ServerInfo',
      'ListTypes',
      'ListDetails',
      'ListSummary',
      'ListValue',
    ];

    // Get BrAPI versions.
    $brapi_versions = brapi_available_versions();

    // @todo Allow the use of a URL parameter to manage a specific version
    // mapping, even if it is not enabled (for export for instance).

    // Get current settings.
    $config = \Drupal::config('brapi.settings');
    $active_definitions = [];
    foreach ($brapi_versions as $version => $version_definition) {
      if ($config->get($version)) {
        $active_definitions[$version] = $config->get($version . 'def');
      }
    }

    // Get data type mapping entities.
    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');

    foreach ($active_definitions as $version => $active_def) {
      if (!empty($active_def) && !empty($brapi_versions[$version][$active_def])) {
        $active_def_id = preg_replace('#\W#', '_', $active_def);
        $form[$active_def_id] = [
          '#type' => 'details',
          '#title' => $this->t($version . ' (%def) data type settings', ['%def' => $active_def]),
          '#open' => FALSE,
          '#tree' => TRUE,
        ];

        $brapi_definition = brapi_get_definition($version, $active_def);
        foreach ($brapi_definition['data_types'] as $datatype => $datatype_definition) {
          if (empty($datatype_definition['calls'])) {
            // Skip datatypes not used in calls.
            continue 1;
          }
          // Skip special datatypes managed internally.
          // Always allows the use of calls: v1/calls, v2/serverinfo,
          // v1 search calls, v2/lists.
          if (
            (('v2' == $version)
              && (in_array($datatype, $v2_internal_types))
            )
            || (('v1' == $version)
              && (in_array($datatype, $v1_internal_types))
            )
          ) {
            continue 1;
          }

          // Generate datatype machine name.
          $datatype_id = brapi_generate_datatype_id($datatype, $version, $active_def);

          $form[$active_def_id][$datatype] = [
            '#type' => 'details',
            '#title' => $datatype,
            '#open' => FALSE,
          ];

          $form[$active_def_id][$datatype]['datatype'] = [
            '#type' => 'hidden',
            '#value' => $datatype,
          ];

          // Display mapping list.
          $mapping = $mapping_loader->load($datatype_id);
          if (empty($mapping)) {
            $form[$active_def_id][$datatype]['action_link'] = [
              '#type' => 'link',
              '#title' => $this->t('Add mapping'),
              '#url' => \Drupal\Core\Url::fromRoute('entity.brapidatatype.add_form', ['mapping_id' => $datatype_id]),
            ];
          }
          else {
            $form[$active_def_id][$datatype]['action_link'] = [
              '#type' => 'link',
              '#title' => $this->t('Edit mapping'),
              '#url' => \Drupal\Core\Url::fromRoute('entity.brapidatatype.edit_form', ['brapidatatype' => $datatype_id]),
            ];
            $form[$active_def_id][$datatype]['#open'] = TRUE;
          }

          $form[$active_def_id][$datatype]['details'] = [
            '#type' => 'markup',
            '#markup' =>
              '<br/>Id: '
              . $datatype_id
              . '<br/>Type: '
              . $datatype_definition['type']
              . '<br/>Calls :'
              . implode(', ', array_keys($datatype_definition['calls']))
              . '<br/>Fields: '
              . implode(', ', array_keys($datatype_definition['fields']))
            ,
          ];
        }

        // Sort datatypes.
        ksort($form[$active_def_id]);

        // Complete mapping.
        $complete_options = [];
        foreach ($brapi_versions as $version => $subversions) {
          $subversion_numbers = array_keys($subversions);
          $complete_options = array_merge($complete_options, array_combine($subversion_numbers, $subversion_numbers));
        }
        unset($complete_options[$active_def]);

        $form[$active_def_id]['complete'] = [
          '#type' => 'container',
          '#weight' => count($brapi_definition['data_types']) + 15,
          '#attributes' => [
            'class' => ['container-inline'],
          ],
        ];
        $form[$active_def_id]['complete']['complete_version'] = [
          '#type' => 'select',
          '#title' => $this->t('Complete %def mapping from version', ['%def' => $active_def]),
          '#weight' => 5,
          '#options' => $complete_options,
        ];
        $form[$active_def_id]['complete']['complete_submit'] = [
          '#type' => 'submit',
          '#value' => $this->t('Complete'),
          '#name' => 'complete_' . $active_def_id,
          '#weight' => 10,
        ];

        // Export mapping.
        // Note: special characters in #name (or "<>" in #value) brings a bug in
        // the use of the correct #submit method: the one defined in the first
        // submit button is used instead.
        $form[$active_def_id]['export'] = [
          '#type' => 'submit',
          '#value' => $this->t('Export :def mapping', [':def' => $active_def]),
          '#name' => 'export_' . $active_def_id,
          '#weight' => count($brapi_definition['data_types']) + 20,
        ];

        // Import mapping.
        $form[$active_def_id]['import'] = [
          '#type' => 'container',
          '#weight' => count($brapi_definition['data_types']) + 25,
          '#attributes' => [
            'class' => ['container-inline'],
          ],
        ];
        $form[$active_def_id]['import']['import_mapping'] = [
          '#type' => 'managed_file',
          '#title' => $this->t('Mapping file to import'),
          '#weight' => 5,
          '#upload_location' => 'private://',
          '#upload_validators' => [
            'file_validate_extensions' => ['json yml yaml'],
          ],
        ];
        $form[$active_def_id]['import']['import_submit'] = [
          '#type' => 'submit',
          '#value' =>  $this->t('Import'),
          '#name' => 'import_' . $active_def_id,
          '#weight' => 10,
        ];

        // Remove all.
        $form[$active_def_id]['delete'] = [
          '#type' => 'submit',
          '#value' => $this->t('Remove all :def mapping', [':def' => $active_def]),
          '#name' => 'delete_' . $active_def_id,
          '#weight' => count($brapi_definition['data_types']) + 30,
        ];

      }
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function buildDeleteConfirmForm(array $form, FormStateInterface $form_state) {

    $verdef = $form_state->get('confirm_delete', ['version' => '', 'definition' => '',]);
    $form_state->set('confirm_delete', FALSE)->setRebuild(TRUE);

    // Make sure we got something to delete.
    if (empty($verdef['definition'])) {
      return $this->buildForm($form, $form_state);
    }

    $form['question'] = [
      '#type' => 'markup',
      '#markup' => $this->t(
        'Are you sure you want to remove BrAPI mapping configuration for version %definition? This operation can not be undone (unless you exported the mapping and reimport it afteward).',
        ['%definition' => $verdef['definition'],]
      ),
    ];

    $form['actions'] = [
      '#type' => 'container',
    ];

    $form['actions']['confirm'] = [
      '#type' => 'submit',
      '#value' => $this->t('Confirm'),
      '#submit' => ['::submitConfirmDeleteForm'],
    ];
    $form['actions']['cancel'] = [
      '#type' => 'submit',
      '#value' => $this->t('Cancel'),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $action = $form_state->getTriggeringElement()['#name'];
    if (preg_match('#^(complete|export|import|delete)_(\d)_(\d.*)#', $action, $matches)) {
      $operation     = $matches[1];
      $version       = 'v' . $matches[2];
      $definition    = $matches[2] . '.' . $matches[3];
      $definition_id = $matches[2] . '_' . $matches[3];
      switch ($operation) {
        case 'complete':
          $this->submitCompleteForm($form, $form_state, $version, $definition, $definition_id);
          break;

        case 'export':
          $this->submitExportForm($form, $form_state, $version, $definition, $definition_id);
          break;

        case 'import':
          $this->submitImportForm($form, $form_state, $version, $definition, $definition_id);
          break;

        case 'delete':
          $this->submitDeleteForm($form, $form_state, $version, $definition, $definition_id);
          break;

        default:
          // Should never get there unless the regex is modified.
          $this->logger('brapi')->warning('Unrecognized operation "%operation".', ['%operation' => $operation,]);
          break;
      }
    }
    else {
      $this->logger('brapi')->warning('Unsupported form action "%action".', ['%action' => $action,]);
    }

    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));
  }

  /**
   * {@inheritdoc}
   */
  public function submitCompleteForm(array &$form, FormStateInterface $form_state, string $version, string $definition, string $definition_id) {
    $this->messenger()->addMessage('Complete not implemented yet.');
    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));
  }

  /**
   * {@inheritdoc}
   */
  public function submitExportForm(
    array &$form,
    FormStateInterface $form_state,
    string $version,
    string $definition,
    string $definition_id
  ) {

    $export_data = [];
    $serializeFieldSettings = function ($field_settings, $current_data = 'mapping') use (&$serializeFieldSettings) {
      if (is_array($field_settings)) {
        $serialized_settings = [];
        foreach ($field_settings as $key => $value) {
          $serialized_settings += $serializeFieldSettings($value, $current_data . '[' . $key . ']');
        }
        return $serialized_settings;
      }
      else {
        return [$current_data => $field_settings];
      }
    };

    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');
    $def_values = $form_state->getValues()[$definition_id];
    foreach ($def_values as $item_name => $item_value) {
      if (is_array($item_value) && !empty($item_value['datatype'])) {
        $datatype = $item_value['datatype'];
        $datatype_id = brapi_generate_datatype_id($datatype, $version, $definition);
        $entity = $mapping_loader->load($datatype_id);
        if (!empty($entity)) {
          $settings = $entity->toArray();
          $exported_item = [
            'id' => $settings['id'],
            'label' => $settings['label'],
            'contentType' => $settings['contentType'],
          ];
          $exported_item += $serializeFieldSettings($settings['mapping']);
          $export_data[$datatype] = $exported_item;
        }
      }
    }
    try {
      $yaml_data = Yaml::encode($export_data);
    }
    catch (InvalidDataTypeException $e) {
    }
    if (!empty($yaml_data)) {
      $response = new Response($yaml_data);
      $disposition = HeaderUtils::makeDisposition(
        HeaderUtils::DISPOSITION_ATTACHMENT,
        'brapi_' . $version . '-' . $definition . '_mapping.yml'
      );
      $response->headers->set('Content-Disposition', $disposition);
      $form_state->setResponse($response);
    }
    else {
      $this->messenger()->addError(
        'Failed to export BrAPI %def mapping.', ['%def' => $definition,]
      );
      if (!empty($e)) {
        $this->logger('brapi')->error(
          'Failed to export BrAPI ' . $definition . ' mapping: ' . $e
        );
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function submitImportForm(
    array &$form,
    FormStateInterface $form_state,
    string $version,
    string $definition,
    string $definition_id
  ) {
    $mapping_loader = \Drupal::service('entity_type.manager')->getStorage('brapidatatype');
    $mapping_file_id = $form_state->getValue($definition_id)['import']['import_mapping'][0] ?? FALSE;
    if ($mapping_file_id) {
      $mapping_file = File::load($mapping_file_id);
      $mapping_data = file_get_contents($mapping_file->getFileUri());
      if ($mapping_data !== FALSE) {
        $this->logger('brapi')->notice('Importing BrAPI mapping for v' . $definition);
        try {
          $mapping = Yaml::decode($mapping_data);
          // Get BrAPI specifications.
          $brapi_definition = brapi_get_definition($version, $definition);

          foreach ($mapping as $datatype => $settings_raw) {
            // Check if the datatype exists in current definition.
            if (!array_key_exists($datatype, $brapi_definition['data_types'])) {
              $log = 'WARNING: Datatype "' . $datatype . '" does not exist in BrAPI v' . $definition . ' specifications. Ignored.';
              $this->messenger()->addWarning($log);
              $this->logger('brapi')->warning($log);
              continue;
            }

            // Check content.
            if (!array_key_exists('contentType', $settings_raw)
              || !array_key_exists('label', $settings_raw)
              || (count($settings_raw) < 3)
            ) {
              $log = 'WARNING: Datatype "' . $datatype . '" definition is incomplete. Ignored.';
              $this->messenger()->addWarning($log);
              $this->logger('brapi')->warning($log);
              continue;
            }
            // Make sure content type exists.
            list($content_type, $bundle) = explode(':', $settings_raw['contentType'] . ':');
            if (empty($content_type) || empty($bundle)) {
              $log = 'WARNING: Datatype "' . $datatype . '" definition has an invalid content type definition :"' . $settings_raw['contentType'] . '". Ignored.';
              $this->messenger()->addWarning($log);
              $this->logger('brapi')->warning($log);
              continue;
            }
            $bundle_info = \Drupal::service('entity_type.bundle.info')->getBundleInfo($content_type);
            if (!array_key_exists($bundle, $bundle_info)) {
              $log = 'WARNING: Datatype "' . $datatype . '" definition is using an unexisting content type "' . $content_type . '" (bundle "' . $bundle . '"). Ignored.';
              $this->messenger()->addWarning($log);
              $this->logger('brapi')->warning($log);
              continue;
            }

            $datatype_id = brapi_generate_datatype_id($datatype, $version, $definition);

            // Extract setting structure from setting string.
            $dt_settings = [
              'id' => $datatype_id,
              'contentType' => $settings_raw['contentType'],
              'label' => preg_replace('#v\d\.\d+#', 'v' . $definition, $settings_raw['label']),
              'mapping' => [],
            ];
            foreach ($settings_raw as $key => $value) {
              if (0 == strncmp($key, 'mapping', 7)) {
                $subkeys = explode('][', substr($key, 8, -1));
                $subvalue = &$dt_settings['mapping'];
                while ($subkey = array_shift($subkeys)) {
                  $subvalue = &$subvalue[$subkey];
                }
                if ('submapping' == $subkey) {
                  $value = preg_replace('#v\d\.\d+#', 'v' . $definition, $subkey);
                }
                $subvalue = $value;
              }
            }

            // Check if a mapping already exists.
            $mapping = $mapping_loader->load($datatype_id);
            if (!empty($mapping)) {
              // Yes, update.
              foreach ($dt_settings as $key => $value) {
                $mapping->set($key, $value);
              }
              $mapping->save();
              $log = 'Replaced datatype mapping for "' . $datatype . '".';
              $this->messenger()->addMessage($log);
              $this->logger('brapi')->notice($log);
            }
            else {
              // No, create a new one.
              $mapping = BrapiDatatype::create($dt_settings);
              $mapping->save();
              $log = 'Added new datatype mapping for "' . $datatype . '".';
              $this->messenger()->addMessage($log);
              $this->logger('brapi')->notice($log);
            }
          }
        }
        catch (InvalidDataTypeException $e) {
          $this->messenger()->addError(
            'Failed to parse BrAPI mapping file for import.'
          );
          $this->logger('brapi')->error(
            'Failed to parse '
            . $mapping_file->getFileUri()
            . 'BrAPI mapping file for import: '
            . $e
          );
        }
      }
      else {
        $this->messenger()->addError('Failed to read mapping file.');
      }
    }
    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));
  }

  /**
   * {@inheritdoc}
   */
  public function submitDeleteForm(array &$form, FormStateInterface $form_state, string $version, string $definition, string $definition_id) {
    $form_state->set(
      'confirm_delete',
      [
        'version' => $version,
        'definition' => $definition,
      ]
    )->setRebuild(TRUE);
  }

  /**
   * {@inheritdoc}
   */
  public function submitConfirmDeleteForm(array &$form, FormStateInterface $form_state) {
    $this->messenger()->addMessage('Delete not implemented yet.');
    // Clear cache.
    \Drupal::cache('brapi_search')->invalidateAll();
    $this->messenger()->addMessage($this->t('BrAPI search cache has been cleared.'));
  }

}
