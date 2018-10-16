/** Initialize the list of check kinds
 *
 * param e - event
 */
function init_check_kinds_list(e) {
  var list = document.getElementById('check_kinds_list');
  for (var i = 0; i < window.check_kinds.length; i++) {
    var check_kind = window.check_kinds[i];

    var button = document.createElement('input');
    button.type = 'checkbox';
    button.checked = window.check_kinds_filter[check_kind.id];
    button.value = check_kind.id;
    button.addEventListener('change', function(e) {
      update_check_kinds_list_value(e);
    });

    var label = document.createElement('label')
    label.appendChild(button);
    label.appendChild(document.createTextNode(check_kind.name));

    var li = document.createElement('li');
    li.appendChild(label);

    list.appendChild(li);
  }
}

/** Callback function called when the user clicks on a check kinds checkbox
 *
 * param e - event
 */
function update_check_kinds_list_value(e) {
  window.check_kinds_filter[e.target.value] = e.target.checked;
  init_files_list();
}

/** Get the URL parameter for the check kinds filter */
function check_kinds_filter_parameter() {
  // Find the maximum kind
  var max_kind = Object.keys(window.check_kinds_filter).reduce(function(a, b) {
    return Math.max(a, b);
  });

  // Build a byte array
  var array = new Uint8Array(Math.floor(max_kind / 8) + 1);
  for (var kind in window.check_kinds_filter) {
    if (window.check_kinds_filter[kind] === true) {
      array[Math.floor(kind / 8)] |= 1 << (kind % 8);
    }
  }

  // Convert to hexadecimal
  return Array.from(array, function(byte) {
    return ('0' + byte.toString(16)).slice(-2).toUpperCase();
  }).join('');
}

/** Initialize the list of files
 *
 * param e - event
 */
function init_files_list(e) {
  var check_kinds_filter_param = check_kinds_filter_parameter();
  var table = document.getElementById('table_files_tbody');
  table.innerHTML = '';
  for (var i = 0; i < window.files.length; i++) {
    var file = window.files[i];

    var a = document.createElement('a');
    a.className = 'file_link';
    a.href = '/report/' + file.id + '?k=' + check_kinds_filter_param;
    a.appendChild(document.createTextNode(file.path));

    var td_name = document.createElement('td');
    td_name.appendChild(a);

    var stats_span = document.createElement('span');
    stats_span.className = 'stats';
    var oks = this.count_checks(file.status_kinds[0]);
    var warnings = this.count_checks(file.status_kinds[1]);
    var errors = this.count_checks(file.status_kinds[2]);
    var unreachables = this.count_checks(file.status_kinds[3]);
    if (warnings === 0 && errors === 0 && unreachables == 0) {
      stats_span.innerHTML = '<span>Safe</span>';
    } else {
      stats_span.innerHTML = '<span class="text_ok">' + oks + '</span>'
                           + '<span class="text_deadcode">' + unreachables + '</span>'
                           + '<span class="text_warning">' + warnings + '</span>'
                           + '<span class="text_error">' + errors + '</span>';
    }

    var td_stats = document.createElement('td');
    td_stats.appendChild(stats_span);

    var tr = document.createElement('tr');
    tr.appendChild(td_name);
    tr.appendChild(td_stats);
    table.appendChild(tr);
  }
}

/** Count the number of checks given the map from kind to count
 *
 * param kinds - the map from kind to count
 */
function count_checks(kinds) {
  var value = 0;
  for (var kind in kinds) {
    if (window.check_kinds_filter[kind] === true) {
      value += kinds[kind];
    }
  }
  return value;
}

/** load event */
window.addEventListener('load', init_check_kinds_list);
window.addEventListener('load', init_files_list);
