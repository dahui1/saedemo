#!env python2

import rpc_pb2
import interface_pb2
import zmq

context = zmq.Context()


def request(server, method, params):
    socket = context.socket(zmq.REQ)
    socket.connect(server)

    request = rpc_pb2.Request()
    request.method = method
    request.param = params

    socket.send(request.SerializeToString())
    reply = socket.recv()
    response = rpc_pb2.Response.FromString(reply)

    return response.data


def pbrequest(server, method, params):
    params = params.SerializeToString()
    response = request(server, method, params)
    return response


class SAEClient(object):
    def __init__(self, endpoint):
        self.endpoint = endpoint

    def echo_test(self, s):
        return request(self.endpoint, "echo_test", s)

    def _entity_search(self, method, query, offset, count):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = ""
        r.query = query
        r.offset = offset
        r.count = count
        response = pbrequest(self.endpoint, method, r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def _entity_detail_search(self, method, ids):
        r = interface_pb2.EntityDetailRequest()
        r.dataset = ""
        r.id.extend(ids)
        response = pbrequest(self.endpoint, method, r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def author_search_by_id(self, dataset, aids):
        return _entity_detail_search("AuthorSearchById", aids);

    def author_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("AuthorSearch", query, offset, count)

    def pub_search_by_author(self, dataset, author_id, offset=0, count=20):
        return self._entity_search("PubSearchByAuthor", str(author_id), offset, count)

    def pub_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("PubSearch", query, offset, count)

    def jconf_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("JConfSearch", query, offset, count)

    def influence_search_by_author(self, dataset, aid):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = str(aid)
        response = pbrequest(self.endpoint, "InfluenceSearchByAuthor", r)
        er = interface_pb2.InfluenceSearchResponse()
        er.ParseFromString(response)
        return er

    def patent_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("PatentSearch", query, offset, count)

    def patent_search_by_group(self, dataset, group_id, offset=0, count=20):
        return self._entity_search("PatentSearchByGroup", str(group_id), offset, count)

    def patent_search_by_inventor(self, dataset, inventor_id, offset=0, count=20):
        return self._entity_search("PatentSearchByInventor", str(inventor_id), offset, count)

    def group_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("GroupSearch", query, offset, count)

    def group_search_by_id(self, dataset, gids):
        return self._entity_detail_search("GroupSearchById", gids);

    def inventor_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("InventorSearch", query, offset, count)

    def influence_search_by_group(self, dataset, gid):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = str(gid)
        response = pbrequest(self.endpoint, "InfluenceSearchByGroup", r)
        er = interface_pb2.InfluenceSearchResponse()
        er.ParseFromString(response)
        return er


    def user_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("UserSearch", query, offset, count)

    def weibo_search(self, dataset, query, offset=0, count=20):
        return self._entity_search("WeiboSearch", query, offset, count)

def main():
    c = SAEClient("tcp://localhost:40112")
    print c.author_search("academic", "data mining")


if __name__ == "__main__":
    main()
