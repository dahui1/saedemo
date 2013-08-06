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

    def author_search_by_id(self, dataset, aids):
        r = interface_pb2.EntityDetailRequest()
        r.dataset = dataset
        r.id.extend(aids)
        response = pbrequest(self.endpoint, "AuthorSearchById", r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def author_search(self, dataset, query, offset=0, count=20):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = query
        r.offset = offset
        r.count = count
        response = pbrequest(self.endpoint, "AuthorSearch", r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def pub_search_by_author(self, dataset, author_id, offset=0, count=20):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = str(author_id)
        r.offset = offset
        r.count = count
        response = pbrequest(self.endpoint, "PubSearchByAuthor", r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def pub_search(self, dataset, query, offset=0, count=20):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = query
        r.offset = offset
        r.count = count
        response = pbrequest(self.endpoint, "PubSearch", r)
        er = interface_pb2.EntitySearchResponse()
        er.ParseFromString(response)
        return er

    def jconf_search(self, dataset, query, offset=0, count=20):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = query
        r.offset = offset
        r.count = count
        response = pbrequest(self.endpoint, "JConfSearch", r)
        er = interface_pb2.EntitySearchResponse()
        er.query = query
        er.total_count = 0
        return er

    def influence_search_by_author(self, dataset, aid):
        r = interface_pb2.EntitySearchRequest()
        r.dataset = dataset
        r.query = str(aid)
        response = pbrequest(self.endpoint, "InfluenceSearchByAuthor", r)
        er = interface_pb2.InfluenceSearchResponse()
        er.ParseFromString(response)
        return er


def main():
    c = SAEClient("tcp://localhost:40112")
    print c.author_search("academic", "data mining")


if __name__ == "__main__":
    main()
