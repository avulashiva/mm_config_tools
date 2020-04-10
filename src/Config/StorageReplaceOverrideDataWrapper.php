<?php
namespace Drupal\mm_config_tools\Config;

use Drupal\config\StorageReplaceDataWrapper;
use Drupal\Core\Config\StorageInterface;

/**
 * @file
 * Contains ${NAMESPACE}\StorageReplaceOverrideDataWrappe
 */
class StorageReplaceOverrideDataWrapper extends StorageReplaceDataWrapper {
  protected $excludePatterns;

  /**
   * @inheritDoc
   */
  public function __construct(StorageInterface $storage, $collection = StorageInterface::DEFAULT_COLLECTION) {
    parent::__construct($storage, $collection);
    $configToolsSettings = $this->storage->read('mm_config_tools.settings')['override_configs'];
    $this->excludePatterns = explode("\r\n", $configToolsSettings);
  }

  /**
   * @inheritDoc
   */
  public function replaceData($name, array $data) {
    if (! $this->replaceProtected($name)) {
      return parent::replaceData($name, $data);
    }
  }

  /**
   * Check if a config name is replace protected.
   *
   * @param $name
   * @return bool
   */
  protected function replaceProtected($name) {
    $protected = FALSE;
    foreach ($this->excludePatterns as $excludePattern) {
      $pattern = '/' . $excludePattern . '/';
      if (preg_match($pattern, $name)) {
        $protected = TRUE;
        break;
      }
    }
    return $protected;
  }

}