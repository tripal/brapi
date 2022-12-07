<?php

namespace Drupal\brapi;

/**
 * Defines an asynchroneous search manager.
 */
class BrapiAsyncSearch {
  
  /**
   * Array of search to perform.
   *
   * @var array
   */
  protected $search_data;

  /**
   * Add a new search to perform.
   */
  public function addSearch(array $search_data) {
    $this->search_data[] = $search_data;
  }

  /**
   * Return all current searches not yet performed.
   */
  public function getSearch() :?array {
    return $this->search_data;
  }

  /**
   * Perform the current searches.
   */
  public function performSearches() {
    while ($this->search_data) {
      $search = array_shift($this->search_data);
      // Perform the search.
      try {
        //@todo: clear pager data
        $search_result = $search['controller']->processObjectCalls(
          $search['request'],
          $search['config'],
          $search['version'],
          $search['call'],
          $search['method']
        );
        // Save search result to cache.
        // @todo: manage search result storage strategies:
        //   save query filters or save resulting identifiers as list
        //   or save the full result set?
        \Drupal::cache('brapi_search')->set(
          $search['cid'],
          $search_result,
          $search['expiration'],
          ['brapi']
        );
      }
      catch (\Symfony\Component\HttpKernel\Exception\NotFoundHttpException $e) {
        \Drupal::logger('brapi')->notice('Deferred search ' . $search['cid'] . ' did not find matches. ' . $e);
        \Drupal::cache('brapi_search')->set(
          $search['cid'],
          ['metadata' => ['code' => 404,]],
          $search['expiration'],
          ['brapi']
        );
      }
      catch (\Exception $e) {
        \Drupal::logger('brapi')->error('Deferred search ' . $search['cid'] . 'failed. ' . $e);
        \Drupal::cache('brapi_search')->set(
          $search['cid'],
          ['metadata' => ['code' => 404,]],
          $search['expiration'],
          ['brapi']
        );
      }
    }
  }

}
