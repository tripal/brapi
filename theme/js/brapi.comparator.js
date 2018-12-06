/**
 * @file
 * BrAPI Javascript library
 *
 */
(function ($) {
"use strict";

  Drupal.brapi = Drupal.brapi || {};

  Drupal.behaviors.brapi = {
    attach: function(context, settings) {
/******************************************************************************/
$(function() {
  var entity_settings = {
    'germplasm': {
      'search': 'germplasm-search',
      'details': 'germplasm/{germplasmDbId}'
    }
  };

  var $settings = $('form#brapi_comparator_settings');

  $('#brapi_comparator input.brapi-comparator-button').click(function () {
    var brapi_comparator_url1 = $('#brapi_comparator_url1').val();
    var brapi_comparator_url2 = $('#brapi_comparator_url2').val();
    alert('Test: ' + brapi_comparator_url1 + ' vs ' + brapi_comparator_url2);
  });
  

});
/******************************************************************************/
    }
  };
})(jQuery);
