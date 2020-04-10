<?php

namespace Drupal\mm_config_tools\Form;

use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Class SettingsForm.
 *
 * @package Drupal\mm_config_tools\Form
 */
class SettingsForm extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  protected function getEditableConfigNames() {
    return [
      'mm_config_tools.settings',
    ];
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'mm_config_tool_settings_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $config = $this->config('mm_config_tools.settings');
    $form['override_configs'] = [
      '#type' => 'textarea',
      '#title' => $this->t('Override configs'),
      '#description' => $this->t('Define patterns which, if they exist in a config, will not be overwritten when config import is done. One regex per line.'),
      '#default_value' => $config->get('override_configs'),
    ];
    return parent::buildForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    parent::validateForm($form, $form_state);
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    parent::submitForm($form, $form_state);

    $this->config('mm_config_tools.settings')
      ->set('override_configs', $form_state->getValue('override_configs'))
      ->save();
  }

}
