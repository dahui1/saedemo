link_types = dict(
    homepage=0,
    google_scholar=1,
    linkedin=2,
    microsoft_academic=3,
    facebook=4,
    twitter=5,
    google_plus=6,
    sina_weibo=7,
    video_lecture=8
)
reverse_link_types = {v: k for k, v in link_types.items()}

from collections import defaultdict
link_maps = defaultdict(dict)
import csv
reader = csv.DictReader(open("/home/thinxer/aminer/author_links.csv"))
for row in reader:
    link_maps[int(row['AUTHORID'])][reverse_link_types[int(row['type'])]] = {
        'url': row['LINK']
    }


def query(aminerid):
    return link_maps[aminerid]
