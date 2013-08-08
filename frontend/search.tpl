<style type="text/css">
	.result-item {
		padding: 4px 2px 4px 6px;
		border-bottom: 1px dashed #f6f6f6;
	}

	.item-img {
		max-width: 120px;
		padding: 2px;
		border: 1px solid #ddd;
		border-radius: 4px;
	}

	.item-img:hover {
		border-color: #0088cc;
	}

	.item-stats {
		color: green;
		font-size: 12px;
	}

	ul.item-stats > li {
		margin-left: 0;
		padding-left: 0;
	}

	.item-description {
		color: grey;
		margin: 0;
	}

	.item-topics > li {
		padding-left: 0;
		font-size: 11px;
		border: solid 1px rgba(99, 99, 99, 0.18);
	}

</style>

<form class="search form-search" method="get">
	<fieldset>
		<legend>Entity Search</legend>
		<input type="text" class="search-query" name="q" placeholder="type in a topic, e.g. data mining" value="{{query}}"/>
		<button class="btn btn-primary" type="submit">Search</button>
		<button class="btn btn-analysis">Knowledge Drifting</button>
	</fieldset>
</form>

%if defined("hotqueries"):
	<p>Hot queries:
	%for query in hotqueries:
	<span><a href="search?q={{query}}">{{query}}</a></span>
	%end
%end

<div class="row-fluid">
	<div class="results span8">
	<div class="results-summary pull-right">{{count}} results, displaying 0 - 15</div>
	<h4>{{get("results_title", "Results")}}</h4>
	<ul class="unstyled">
	%for item in results:
		<li class="result-item row-fluid">
			<div class="item-img span2">
				<a href="{{item['url']}}">
					<img src="{{item['imgurl']}}" alt="{{item['name']}}" style="width: 100%; height: auto;"/>
				</a>
			</div>
			<div class="item-content span10">
				<div class="item-name">
					<a href="{{item['url']}}">{{item['name']}}</a>
					%if 'integrated' in item:
						%for k, v in item['integrated'].items():
						<span>[<a href="{{v['url']}}">{{k}}</a>]</span>
						%end
					%end
					<span class="pull-right">[<a href="{{item['id']}}/influence">Influence Analysis</a>]</span>
				</div>
				<ul class="item-stats inline">
				%for k, v in item['stats'].items():
					<li>{{k}}: <b><i>{{v}}</i></b></li>
				%end
				</ul>
				<p class='item-description'>{{item['description']}}</p>
				<ul class="item-topics inline">
				%for t in item['topics']:
					<li>{{t}}</li>
				%end
				</ul>
			</div>
		</li>
	%end
	</ul>
	</div>
	%if defined("extra_results_list"):
	<div class="extra-results-pane span4">
		%for extra_results in extra_results_list:
		<div class="extra-results">
			<section>
			<h4>{{extra_results['title']}}</h4>
			<ul>
			%for item in extra_results['items']:
				<li><a href="{{item['link']}}">{{item['text']}}</a></li>
			%end
			</ul>
			</section>
		</div>
		%end
	</div>
	%end
</div>

<script type="text/javascript">
	$('.btn-analysis').click(function() {
		var query = $('.search-query', $(this).parent()).val();
		window.location = "topictrends?q=" + encodeURIComponent(query);
		return false;
	});
	$(document).ready(function() {
		$('.result-item .item-img img').one('error', function() {
			$(this).attr('src', 'http://static02.linkedin.com/scds/common/u/img/icon/icon_no_company_logo_100x60.png');
		});
	});
</script>

%rebase layout
