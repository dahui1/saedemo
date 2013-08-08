<style type="text/css">
    .topic-list li {
        cursor: pointer;
    }
	.influence-pie {
        font: 10px sans-serif;
    }
</style>
<div class="row-fluid topic">
    <div class="span4">
        <ul class="topic-list nav nav-tabs nav-stacked">
            %counter = 0
            %for topic in topics:
                <li data-index="{{counter}}"><a>{{topic['topic']}}</a></li>
                %counter += 1
            %end
        </ul>
		
            <div class="influence-pie" style="height: 250px; width: 250px;margin:0 auto"></div>
       
    </div>
    <div class="span8">
        %counter = 0
        %for topic in topics:
        <div class="topic-analysis index{{counter}}" style="{{"display:none" if counter != 0 else ""}}">
			<div class="span4">
            <p> Influenced by {{name}}:
            <ul>
            %for influencee in topic["influencees"]:
                <li><a href="{{influencee[2]}}">{{influencee[0]}}</a>  </li>
            %end
            </ul>
            <p> Influencers:
            <ul>
            %for influencer in topic["influencers"]:
                <li><a href="{{influencer[2]}}">{{influencer[0]}}</a>  </li>
            %end
            </ul>
			</div>
			<div class="table span4"></div>
            %counter += 1
        </div>
        %end
    </div>
</div>


