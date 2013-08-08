#!env python2

import sys
import os.path
from bottle import route, run, template, view, static_file, request, urlencode
from saeclient import SAEClient
import logging

from knowledge_drift import KnowledgeDrift
import influence_analysis
import time
import json

logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)
client = SAEClient("tcp://127.0.0.1:40114")
knowledge_drift_client = KnowledgeDrift()

ask_influ=influence_analysis.asker(client)
ask_tre=influence_analysis.asker_t(client)
ask_table=influence_analysis.asker_table(client)

logging.info("done")

@route('/')
def index():
    return template('index')


@route('/academic/search')
@view('search')
def search():
    q = request.query.q or ''
    print 'searching', q, 'in academic'
    result = client.author_search("", q, 0, 20)
    pub_result = client.pub_search("", q, 0, 20)

    return dict(
        query=q,
        encoded_query=urlencode({"q": result.query}),
        count=result.total_count,
        results_title='Experts',
        results=[
            dict(
                id=e.id,
                name=e.title,
                url="http://arnetminer.org/person/-%s.html" % e.original_id,
                description=e.description,
                stats=dict(
                    (s.type, s.value) for s in e.stat
                ),
                topics=e.topics.split(','),
                imgurl=e.imgurl
            ) for e in result.entity
        ],
        extra_results_list=[
            dict(
                title="Publications",
                items=[
                    dict(
                        text=pub.title,
                        link="http://arnetminer.org/publication/-%s.html" % pub.original_id
                    ) for pub in pub_result.entity
                ]
            ),
        ]
    )


@route('/patent/search')
@view('search')
def search():
    q = request.query.q or ''
    print 'searching', q, 'in patent'
    result = client.group_search("", q, 0, 20)
    pub_result = client.patent_search("", q, 0, 20)

    return dict(
        query=q,
        encoded_query=urlencode({"q": result.query}),
        count=result.total_count,
        results_title='Companies',
        results=[
            dict(
                id=e.id,
                name=e.title,
                url="http://pminer.org/company.do?m=viewCompany&id=%s" % e.original_id,
                description=e.description,
                stats=dict(
                    (s.type, s.value) for s in e.stat
                ),
                topics=e.topics.split(','),
                imgurl=e.imgurl
            ) for e in result.entity
        ],
        extra_results_list=[
            dict(
                title="Patents",
                items=[
                    dict(
                        text=pub.title,
                        link="http://pminer.org/patent.do?m=viewPatent&id=%s" % pub.original_id
                    ) for pub in pub_result.entity
                ]
            ),
        ]
    )


@route('/weibo/search')
@view('search')
def search():
    q = request.query.q or ''
    print 'searching', q, 'in weibo'
    result = client.weibo_search("", q, 0, 20)
    pub_result = client.user_search("", q, 0, 20)

    return dict(
        query=q,
        encoded_query=urlencode({"q": result.query}),
        count=result.total_count,
        results_title='Weibo',
        results=[
            dict(
                id=e.id,
                name=e.title,
                url="http://weibo.com/%s" % e.original_id,
                description=e.description,
                stats=dict(
                    (s.type, s.value) for s in e.stat
                ),
                topics=e.topics.split(','),
                imgurl=e.imgurl
            ) for e in result.entity
        ],
        extra_results_list=[
            dict(
                title="Users",
                items=[
                    dict(
                        text=pub.title,
                        link="http://weibo.com/u/%s" % pub.original_id
                    ) for pub in pub_result.entity
                ]
            ),
        ]
    )


@route('/<data>/topictrends')
@view('knowledge_drift')
def search(data):
    q = request.query.q or ''
    print 'rendering trends for', q, 'on', data
    knowledge_drift_client.data_set = data
    return dict(
        query=q
    )

@route('/<data>/terms')
def search(data):
    q = request.query.q or ''
    start = int(request.query.start) or 0
    end = int(request.query.end) or 10000
    print 'rendering terms for', q, 'on', data, 'between', start, "and", end
    return knowledge_drift_client.query_terms(q, start_time=start, end_time=end)

@route('/<data>/render')
def topic_trends(data):
    q = request.query.q or ''
    threshold = request.query.threshold or ''
    print 'rendering trends for', q, threshold, 'on', data
    return knowledge_drift_client.query_topic_trends(q, float(threshold))

@route('/<data>/<uid:int>/influence/trends')
def influence_trends(data, uid):
    tmp_idd=int()
    tmp_idd=uid
    return json.dumps(ask_tre.ask(tmp_idd))
    

@route('/<data>/<uid:int>/influence/miserable')
def influence_table(data,uid):
    #pass
    tmp_id=int()
    tmp_id=uid
    da=ask_table.ask(tmp_id)
    return json.dumps(da)

@route('/<data>/<uid:int>/influence/paper')
def influence_paper(data,uid):
    #data = [{"label":"data mining", "value":20}, 
     #     {"label":"XML data", "value":50}, 
      #    {"label":"Information Retrieval", "value":30}];
    tmp_id=int()
    tmp_id=uid
    return json.dumps(ask_influ.ask_pie(tmp_id))
    #return json.dumps(data)


@route('/<data>/<uid:int>/influence/topics/<date>')
@view('influence_topics')
def influence_topics(data, uid, date):
    tmp_id=int()
    tmp_id=uid
    return ask_influ.ask(tmp_id)


@route('/<data>/<uid:int>/influence')
@view('influence')
def influence(data, uid):
    result=client.author_search_by_id("",[uid])
    influence_index=dict(
        name=result.entity[0].title,
        imgurl=result.entity[0].imgurl
)
    return influence_index


@route('/static/<path:path>')
def static(path):
    curdir = os.path.dirname(os.path.realpath(__file__))
    return static_file(path, root=curdir + '/static/')

if len(sys.argv) > 1:
    port = int(sys.argv[1])
else:
    port = 8083

run(server='auto', host='0.0.0.0', port=port, reloader=True, debug=True)
