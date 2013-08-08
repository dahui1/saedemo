#!env python2

import sys
import os.path
from bottle import route, run, template, view, static_file, request, urlencode
from saeclient import SAEClient
import logging

import network_integration
from knowledge_drift import KnowledgeDrift
import influence_analysis
import influence_analysis_patent
import sample_data
import time
import json

logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)
client = SAEClient("tcp://127.0.0.1:40115")
knowledge_drift_client = KnowledgeDrift(client)

ask_influ=influence_analysis.asker(client)
ask_tre=influence_analysis.asker_t(client)
ask_table=influence_analysis.asker_table(client)
ask_influ_p=influence_analysis_patent.asker_p(client)
ask_tre_p=influence_analysis_patent.asker_t_p(client)
ask_table_p=influence_analysis_patent.asker_table_p(client)

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
        encoded_query=urlencode({"q": result.query.encode('utf8')}),
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
                imgurl=e.imgurl,
                integrated=network_integration.query(e.original_id)
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
        encoded_query=urlencode({"q": result.query.encode('utf8')}),
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
    result = client.user_search("", q, 0, 20)
    pub_result = client.weibo_search("", q, 0, 20)

    return dict(
        query=q,
        encoded_query=urlencode({"q": result.query.encode('utf8')}),
        count=result.total_count,
        results_title='Users',
        results=[
            dict(
                id=e.id,
                name=e.title,
                url="http://weibo.com/u/%s" % e.url,
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
                title="Weibo",
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

@route('/academic/<uid:int>/influence/trends')
def influence_trends( uid):
    tmp_idd=int()
    tmp_idd=uid
    return json.dumps(ask_tre.ask(tmp_idd))

@route('/patent/<uid:int>/influence/trends')
def influence_trends_p( uid):
    tmp_idd=int()
    tmp_idd=uid
    return json.dumps(ask_tre_p.ask(tmp_idd))
    

@route('/academic/<uid:int>/influence/miserable')
def influence_table(uid):
    tmp_id=int()
    tmp_id=uid
    da=ask_table.ask(tmp_id)
    return json.dumps(da)

@route('/patent/<uid:int>/influence/miserable')
def influence_table_p(uid):
    tmp_id=int()
    tmp_id=uid
    da=ask_table_p.ask(tmp_id)
    return json.dumps(da)

@route('/academic/<uid:int>/influence/paper')
def influence_paper(uid):
    tmp_id=int()
    tmp_id=uid
    return json.dumps(ask_influ.ask_pie(tmp_id))

@route('/patent/<uid:int>/influence/paper')
def influence_paper(uid):
    tmp_id=int()
    tmp_id=uid
    return json.dumps(ask_influ_p.ask_pie(tmp_id))

@route('/academic/<uid:int>/influence/topics/<date>')
@view('influence_topics')
def influence_topics( uid,date):
    tmp_id=int()
    tmp_id=uid
    return ask_influ.ask(tmp_id)

@route('/patent/<uid:int>/influence/topics/<date>')
@view('influence_topics')
def influence_topics_p( uid,date):
    tmp_id=int()
    tmp_id=uid
    return ask_influ_p.ask(tmp_id)

@route('/academic/<uid:int>/influence')
@view('influence')
def influence(uid):
    result=client.author_search_by_id("",[uid])
    influence_index=dict(
        name=result.entity[0].title,
        imgurl=result.entity[0].imgurl
)
    return influence_index

@route('/patent/<uid:int>/influence')
@view('influence')
def influence(uid):
    return sample_data.influence_index

@route('/static/<path:path>')
def static(path):
    curdir = os.path.dirname(os.path.realpath(__file__))
    return static_file(path, root=curdir + '/static/')

if len(sys.argv) > 1:
    port = int(sys.argv[1])
else:
    port = 8082

run(server='auto', host='0.0.0.0', port=port, reloader=True, debug=True)
