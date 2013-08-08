class asker_p():

    def __init__(self, client):
        self.a=int()
        self.tname=["" for x in range(1000)]
        self.read_topic()
        self.client = client

    def ask_pie(self,id):
        pub=self.client.influence_search_by_group("",id)
        result1=pub.influence
        score=[{}for x in range(1000)]
        for i in range(1000):
            score[i]['score']=0
            score[i]['topic']=0
        for i in result1:
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
        score.sort(key=lambda x:x['score'],reverse=True);
        top_pie=[]
        for i in range(0,5):
            top_id=score[i]['topic']
            tot=score[i]['score']
            top_pie_t={}
            tmp=self.tname[int(top_id)].split(':')
            top_pie_t['label']=tmp[0]
            top_pie_t['value']=tot
            top_pie.append(top_pie_t)
        return top_pie

    def read_topic(self):
        fileHandle = open ('usclassTitle.txt')
        fileList = fileHandle.readlines()
        i=0
        for fileLine in fileList:
            words=fileLine.split('\t')
            top=int(words[0])
            tmp=words[1].split('\n')
            self.tname[top]=tmp[0]
        fileHandle.close()

    def ask(self, id):
        a=int()
        a=id
        pub=self.client.influence_search_by_group("",a)
        result1=pub.influence
        result2=pub.influenced_by
        score=[{}for x in range(1000)]
        for i in range(1000):
            score[i]['score']=0
            score[i]['topic']=0
        influence=[[]for x in range(1000)]
        influenced_by=[[]for x in range(1000)]
        num_influence=[0 for x in range(1000)]
        num_influenced_by=[0 for x in range(1000)]
        for i in result1:
            influence[i.topic].append(i)
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
            num_influence[i.topic]+=1
        score.sort(key=lambda x:x['score'],reverse=True);
        for i in result2:
            influenced_by[i.topic].append(i)
            num_influenced_by[i.topic]+=1
        top_list=[]
        for i in range(0, 5):
            top_id=score[i]['topic']
            topics = {}
            topics['influencers']=[]
            topics['influencees']=[]
            tmp=self.tname[int(top_id)].split(':')
            topics['topic']=tmp[0]
            influence[top_id].sort(key=lambda x:x.score,reverse=True)
            influenced_by[top_id].sort(key=lambda x:x.score,reverse=True)
            num_1=min(5,num_influence[top_id])
            num_2=min(5,num_influenced_by[top_id])
            for j in range(0,num_1):
                temp=[]
                result=self.client.group_search_by_id("",[influence[top_id][j].id])
                temp.append(result.entity[0].title)
                #temp.append(influence[top_id][j].id)
                temp.append(influence[top_id][j].score)
		url="http://www.pminer.org/company.do?m=viewCompany&id="
		url+=str(result.entity[0].original_id)
		temp.append(url)
                tmp_t=tuple(temp)
                topics['influencees'].append(tmp_t)
            for j in range(0,num_2):
                if (influenced_by[top_id][j].id==a):
                    continue
                temp=[]
                result=self.client.group_search_by_id("",[influenced_by[top_id][j].id])
                temp.append(result.entity[0].title)
                #temp.append(influenced_by[top_id][j].id)
                temp.append(influenced_by[top_id][j].score)
                url="http://www.pminer.org/company.do?m=viewCompany&id="
		url+=str(result.entity[0].original_id)
		temp.append(url)
                tmp_t=tuple(temp)
                topics['influencers'].append(tmp_t)
            top_list.append(topics)
        result=self.client.group_search_by_id("",[a])
        final=dict(topics=top_list, name=result.entity[0].title)
        #final=dict(topics=top_list, name=str(a))
        return final

class asker_t_p():

    def __init__(self, client):
        self.a=int()
        self.client = client

    def ask(self, id):
        self.a=id
        pub_result=self.client.patent_search_by_group("",self.a)
        num=[0 for x in range(10000)]
        pub=[[]for x in range(3000)]
        for item in pub_result.entity:
            year=item.stat[0].value
            pub[year].append(item)
            num[year]+=1
        trend=[]
        for i in range(1,3000):
            if num[i]>0:
                tmp={}
                tmp['date']=str(i)
                tmp['value']=num[i]
                tmp['pap1']=pub[i][0].title
                tmp['au1']=""
		tmp['pap2']=0
                tmp['au2']=""
                if num[i]>1:
                    tmp['pap2']=pub[i][1].title
                trend.append(tmp)
        return trend

class asker_table_p():
    
    def __init__(self, client):
        self.client = client

    def ask(self,id):
        a=int()
        a=id
        pub=self.client.influence_search_by_group("",a)
        result1=pub.influence
        result2=pub.influenced_by
        score=[{}for x in range(1000)]
        for i in range(1000):
            score[i]['score']=0
            score[i]['topic']=0
            score[i]['num']=0
        influence=[[]for x in range(1000)]
        influenced_by=[[]for x in range(1000)]
        num_influenced_by=[0 for x in range(1000)]
        num_influence=[0 for x in range(1000)]
        for i in result1:
            influence[i.topic].append(i)
            score[i.topic]['score']+=i.score
            score[i.topic]['topic']=i.topic
            num_influence[i.topic]+=1
        score.sort(key=lambda x:x['score'],reverse=True);
        for i in result2:
            influenced_by[i.topic].append(i)
            num_influenced_by[i.topic]+=1
        table_list=[]
        num=[0 for x in range(1000)]
        peo=[[0 for x in range(1000)] for y in range(1000)]
        for i in range(0, 5):
            top_id=score[i]['topic']
            influence[top_id].sort(key=lambda x:x.score,reverse=True)
            influenced_by[top_id].sort(key=lambda x:x.score,reverse=True)
            num_1=min(5,num_influence[top_id])
            num_2=min(5,num_influenced_by[top_id])
            num[i]=num_1+num_2+1
            table={}
            table['nodes']=[]
            table['links']=[]
            for j in range(0,num_1):
                peo[i][j]=influence[top_id][j].id
            for j in range(0,num_2):
                peo[i][j+num_1]=influenced_by[top_id][j].id
            peo[i][num[i]-1]=a
            for j in range(0,num[i]):
                tmp={}
                tmp_id=peo[i][j]
                result=self.client.group_search_by_id("",[tmp_id])
                tmp['name']=result.entity[0].title
                #tmp['name']=str(tmp_id)
                tmp['group']=2
                table['nodes'].append(tmp)
            for j in range(0,num[i]):
                pub=self.client.influence_search_by_group("",peo[i][j])
                for k in range(0,num[i]):
                    if j==k:
                        continue
                    tmp={}
                    tmp['source']=j
                    tmp['target']=k 
                    weight=0.0
                    for l in pub.influence:
                        if l.topic==top_id and l.id==peo[i][k]:
                            weight=l.score
                            break
                    if weight==0:
                        continue
                    tmp['value']=weight
                    table['links'].append(tmp)
            table_list.append(table)
        return table_list
    
#from saeclient import *
#client = SAEClient("tcp://10.1.1.111:40115")
#p=asker_table_p(client)
#p.ask(450650)
#print p.ask(450650)
