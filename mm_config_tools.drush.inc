<?php
/**
 * @file
 * Command-line tools for safely importing configs.
 */
use Drupal\Core\Config\FileStorage;
use Drupal\mm_config_tools\Config\StorageReplaceOverrideDataWrapper;
use Drush\Config\StorageWrapper;
use Drupal\Core\Config\StorageComparer;
use Drush\Log\LogLevel;

/**
 * Implements hook_drush_command().
 */
function mm_config_tools_drush_command() {
  $items = [];
  $items['config-tools-import'] = [
    'description' => 'Import config from a config directory while leaving overrides.',
    'arguments' => [
      'label' => "A config directory label (i.e. a key in \$config_directories array in settings.php). Defaults to 'sync'",
    ],
    'options' => [
      'preview' => [
        'description' => 'Format for displaying proposed changes. Recognized values: list, diff. Defaults to list.',
        'example-value' => 'list',
      ],
      'source' => [
        'description' => 'An arbitrary directory that holds the configuration files. An alternative to label argument',
      ],
      'partial' => [
        'description' => 'Allows for partial config imports from the source directory. Only updates and new configs will be processed with this flag (missing configs will not be deleted).',
      ],
      'skip-modules' => 'A list of modules to ignore during import (e.g. to avoid disabling dev-only modules that are not enabled in the imported configuration).',
    ],
    'core' => ['8+'],
    'examples' => [
      'drush config-import --skip-modules=devel' => 'Import configuration; do not enable or disable the devel module, regardless of whether or not it appears in the imported list of enabled modules.',
    ],
    'aliases' => ['ctim'],
  ];
  return $items;
}

/**
 * Command callback. Import from specified config directory (defaults to sync).
 */
function drush_mm_config_tools_config_tools_import($source = NULL) {
  global $config_directories;

  // Determine source directory.
  if ($target = drush_get_option('source')) {
    $source_dir = $target;
  }
  else {
    $choices = drush_map_assoc(array_keys($config_directories));
    unset($choices[CONFIG_ACTIVE_DIRECTORY]);
    if (!isset($source) && count($choices) >= 2) {
      $source= drush_choice($choices, 'Choose a source.');
      if (empty($source)) {
        return drush_user_abort();
      }
    }
    elseif (!isset($source)) {
      $source = CONFIG_SYNC_DIRECTORY;
    }
    $source_dir = config_get_config_directory($source);
  }

  /** @var \Drupal\Core\Config\StorageInterface $active_storage */
  $active_storage = \Drupal::service('config.storage');
  $source_storage = new StorageReplaceOverrideDataWrapper($active_storage);
  $file_storage = new FileStorage($source_dir);
  foreach ($file_storage->listAll() as $name) {
    $data = $file_storage->read($name);
    $source_storage->replaceData($name, $data);
  }

  // If our configuration storage is being filtered, then attach all filters
  // to the source storage object.  We will use the filtered values uniformly
  // for comparison, full imports, and partial imports.
  $storage_filters = drush_config_get_storage_filters();
  if (!empty($storage_filters)) {
    $source_storage = new StorageWrapper($source_storage, $storage_filters);
  }

  /** @var \Drupal\Core\Config\ConfigManagerInterface $config_manager */
  $config_manager = \Drupal::service('config.manager');
  $storage_comparer = new StorageComparer($source_storage, $active_storage, $config_manager);


  if (!$storage_comparer->createChangelist()->hasChanges()) {
    return drush_log(dt('There are no changes to import.'), LogLevel::OK);
  }

  if (drush_get_option('preview', 'list') == 'list') {
    $change_list = [];
    foreach ($storage_comparer->getAllCollectionNames() as $collection) {
      $change_list[$collection] = $storage_comparer->getChangelist(NULL, $collection);
    }
    _drush_print_config_changes_table($change_list);
  }
  else {
    // Copy active storage to the temporary directory.
    $temp_dir = drush_tempdir();
    $temp_storage = new FileStorage($temp_dir);
    $source_dir_storage = new FileStorage($source_dir);
    foreach ($source_dir_storage->listAll() as $name) {
      if ($data = $active_storage->read($name)) {
        $temp_storage->write($name, $data);
      }
    }
    drush_shell_exec('diff -x %s -u %s %s', '*.git', $temp_dir, $source_dir);
    $output = drush_shell_exec_output();
    drush_print(implode("\n", $output));
  }

  if (drush_confirm(dt('Import the listed configuration changes?'))) {
    return drush_op('_drush_config_import', $storage_comparer);
  }
}
