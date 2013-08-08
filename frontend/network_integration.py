linkedin_map = {}
with open("/home/thinxer/aminer/aminer_linkedin.txt") as f:
    for l in f:
        aminer_id, inid, inurl = l.strip('\r\n').split('\t')
        linkedin_map[int(aminer_id)] = (inid, inurl)

def query(aminerid):
    ret = {}

    inid, inurl = linkedin_map.get(aminerid, (None, None))
    if inurl:
        ret['linkedin'] = {
            'url': inurl
        }

    return ret
