<?php

namespace Drupal\brapi\Form;

use Drupal\Core\Form\FormStateInterface;

/**
 * Class BrapiDatatypeEditForm.
 *
 * Provides the edit form for our BrAPI Datatype Mapping entity.
 *
 * @ingroup brapi
 */
class BrapiDatatypeEditForm extends BrapiDatatypeFormBase {

  /**
   * {@inheritdoc}
   */
  protected function actions(array $form, FormStateInterface $form_state) {
    $actions = parent::actions($form, $form_state);
    $actions['submit']['#value'] = $this->t('Update BrAPI Datatype Mapping');
    return $actions;
  }

}
