#!env python2

import sys
import os.path
from bottle import route, run, template, view, static_file, request, urlencode
from saeclient import SAEClient
import logging

import sample_data
from knowledge_drift import KnowledgeDrift

logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)
client = SAEClient("tcp://127.0.0.1:40114")
knowledge_drift_client = KnowledgeDrift()
logging.info("done")

@route('/')
def index():
    return template('index')


@route('/<dataset>/search')
@view('search')
def search(dataset):
    q = request.query.q or ''
    print 'searching', q, 'in academic'
    result = client.author_search(dataset, q, 0, 20)
    pub_result = client.pub_search(dataset, q, 0, 20)

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
    return dict(
        query=q,
        count=0,
        results=[],
        encoded_query=urlencode({"q": q})
    )


@route('/weibo/search')
@view('search')
def search():
    q = request.query.q or ''
    print 'searching', q, 'in weibo'
    return dict(
        query=q,
        count=0,
        results=[],
        encoded_query=urlencode({"q": q})
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

@route('/<data>/<uid:int>/influence/trends.tsv')
def influence_trends(data, uid):
    return open('static/influence.tsv')


@route('/<data>/<uid:int>/influence/topics/<date>')
@view('influence_topics')
def influence_topics(data, uid, date):
    # TODO return topics for the given data
    return sample_data.influence_topics


@route('/<data>/<uid:int>/influence')
@view('influence')
def influence(data, uid):
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
