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
  var $settings = $('form#brapi_query_settings');
  // Adds call selection dropdown.
  var $select = $('<select/>')
    .on('change', function() {
      $('form.brapi-query').hide();
      $(this).find('option:selected').data('form').show();
    })
  ;
  $('#brapi_query_result_title, #brapi_query_results')
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .hide();

  var $calls = $('#brapi_query_calls')
    .find('h4')
    .first()
    .after($('<caption class="brapi-call-select">Select a BrAPI call: </caption>').append($select));
  // Hides all the form by default.
  $('form.brapi-query')
    .hide()
    .submit(function(event) {
      event.preventDefault();
      var $form = $(this);
      // Check and fix URL (make sure we got http/https and no trailing '/' on
      // the service URL).
      var service_url = $('#brapi_query_url').val() + $select.val();
      if (!service_url.match(/^https?:\/\//i)) {
        alert('Invalid BrAPI Service URL!');
        $('#brapi_query_url').focus();
        return false;
      }
      service_url = service_url.replace(/\/+$/, '');
      // Add URL argument.
      $form.find('.brapi-query-argument input').each(function(index, item) {
        service_url += '/' + $(item).val();
      });
      // Add query string.
      var query_string = $('<form/>').append($form.find('.barpi-query-string').clone()).serialize();
      var $pretty = $form.find('input[name="pretty"]');
      if ($pretty.length && !$pretty.is(':checked')) {
        query_string = (query_string ? query_string + '&pretty=0' : 'pretty=0');
      }
      service_url += '?' + query_string;
      $form.find('.brapi-query-filter-get input, .brapi-query-filter-get select').each(function(index, item) {
        if ('' != $(item).val()) {
          service_url += '&' + $(item).attr('name') + '=' + encodeURI($(item).val());
        }
      });
      // Update form action URL.
      $form.attr('action', service_url);
      // Get filter data.
      var filter_data = {};
      $form.find('.brapi-query-filter-post input, .brapi-query-filter-post select').each(function(index, item) {
        if ('' != $(item).val()) {
          filter_data[$(item).attr('name')] = $(item).val();
        }
      });
      if ($.isEmptyObject(filter_data)) {
        filter_data = null;
      }
      else {
        filter_data = JSON.stringify(filter_data, null, '\t');
      }
      // Check for debug mode.
      var $debug = $form.find('input[name="debug"]:checked');
      var $debug_zone = $('#brapi_query_debug');
      if ($debug.length) {
        if (!$debug_zone.length) {
          $debug_zone = $('<div id="brapi_query_debug"></div>')
            .insertBefore('#brapi_query_result_title');
          $debug_zone
            .append('<h4 id="brapi_query_debug_title">Data sent</h4>')
            .append('<div id="brapi_query_debug_url"></div>')
            .append('<code></code>')
          ;
        }
        $debug_zone.find('#brapi_query_debug_url').html('URL: ' + service_url);
        $debug_zone.find('code').html(filter_data);
        $debug_zone.show();
      }
      else {
        $debug_zone.hide();
      }


      // Clear output and display animated gif.
      var brapi_image_path = Drupal.settings.brapi.imagePath;
      $('#brapi_query_results')
        .html('<img src="' + brapi_image_path + 'loading_icon.gif" title="Loading, please wait..." alt="Loading..."/>')
        .show();
      // Replace submit by an AJAX call.
      $.ajax({
        url: service_url,
        type: filter_data?'POST':'GET',
        data: filter_data,
        dataType: 'text',
        success: function(output) {
          $('#brapi_query_result_title').show();
          $('#brapi_query_results').show().html(output.trim());
        },
        error: function(jqXHR, textStatus, errorThrown) {
          alert('Failed to run BrAPI call: ' + textStatus);
          $('#brapi_query_results').show().html(textStatus);
        }
      });
    })
    .each(function (index, element) {
      $('<option>' + $(element).find('legend .brapi-call-name').text() + '</option>')
        .appendTo($select)
        .data('form', $(element));
    }
  );
  $select.find('option:selected').data('form').show();

  $("#brapi_query_calls .brapi-datepicker input")
    .not('.brapi-processed')
    .addClass('brapi-processed')
    .datepicker();
});
/******************************************************************************/
    }
  };
})(jQuery);
