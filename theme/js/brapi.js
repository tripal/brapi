/**
 * @file
 * BrAPI Javascript library
 *
 */
(function ($/*, Drupal, window, document, undefined*/) {
"use strict";

  Drupal.brapi = Drupal.brapi || {};

  Drupal.behaviors.brapi = {
    attach: function(context, settings) {
/******************************************************************************/
$(function() {
    
  // Auto-process page BrAPI queries: display matching accession name.
  $('form.brapi-autoquery')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .each(function(index, item) {
      var $brapi_form = $(item);
      var filter_data = {};
      $brapi_form.find('.brapi-query-filter-post input, .brapi-query-filter-post select').each(function(filter_index, filter_item) {
        if ('' != $(filter_item).val()) {
          filter_data[$(filter_item).attr('name')] = $(filter_item).val();
        }
      });
      if ($.isEmptyObject(filter_data)) {
        filter_data = null;
      }
      else {
        filter_data = JSON.stringify(filter_data, null, '\t');
      }
      // get from Ajax
      $.ajax({
        url: $brapi_form.attr('action'),
        type: $brapi_form.attr('method'),
        data: filter_data,
        dataType: 'json',
        success: function(output) {
          if (output
              && output.result
              && output.result.data
              && output.result.data.length
              && output.result.data[0].germplasmName) {
            $brapi_form.html(
              '<a href="https://musabase.org/stock/'
              + output.result.data[0].germplasmDbId
              + '/view" title="Link to MusaBase data">'
              + output.result.data[0].germplasmName
              + '</a>'
            );
          }
          else {
            $brapi_form.html('n/a');
          }
        },
        error: function(jqXHR, textStatus, errorThrown) {
          $brapi_form.html('n/a (error)');
          // alert('Failed to run BrAPI query: ' + textStatus);
        }
      });
    })
  ;
  
});
/******************************************************************************/
    }
  };
})(jQuery);
