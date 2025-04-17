#Data types
```c++
      nmco::Time assessmentStartTime;
      std::string findingUniqueId; // prowler-provider-checkid-accountid-region-resourceid
      std::string provider;
      std::string profile;
      std::string accountId;
      std::string organizationsInfo;
      std::string region;
      std::string checkId;
      std::string checkTitle;
      std::string checkTypes;
      std::string serviceName;
      std::string subServiceName;
      std::string status;
      std::string statusExtended;
      std::string severity;
      std::string resourceId;
      std::string resourceArn;
      std::string resourceTags;
      std::string resourceType;
      std::string resourceDetails;
      std::string description;
      std::string risk;
      std::string relatedUrl;
      std::string recommendation;
      std::string recommendationUrl;
      std::string remediationCode;
      std::string categories;
      std::string notes;
      std::string compliance;
```

#YAML
```yaml
version2:
	assessmentStartTime: ".Timestamp" # Format: "%Y-%m-%dT%H:%M:%SZ"
	findingUniqueId:
	provider: 'aws'
	profile:
	accountId: ".Account Number"
	organizationsInfo:
	region: ".Region"
	checkId: ".Control ID"
	checkTitle: null
	checkTypes:
	serviceName: ".Service"
	subServiceName: null
	status: ".Status"
	statusExtended: null
	severity: ".Severity"
	resourceId: ".Resource ID"
	resourceArn:
	resourceTags:
	resourceType:
	resourceDetails:
	description: ".Control"
	risk: ".Risk"
	relatedUrl: null
	recommendation: ".Remediation"
	recommendationUrl: ".Doc link"
	remediationCode: null
	categories:
	notes:
	compliance: ".Level"

version3:
	assessmentStartTime: ".AssessmentStartTime" # Format: "%Y-%m-%dT%H:%M:%S"
	findingUniqueId: ".FindingUniqueId"
	provider: ".Provider"
	profile: ".Profile"
	accountId: ".AccountId"
	organizationsInfo: ".OrganizationsInfo.{}"
	region: ".Region"
	checkId: ".CheckID"
	checkTitle: ".CheckTitle"
	checkTypes: ".CheckType.[]" # list of strings
	serviceName: ".ServiceName"
	subServiceName: ".SubServiceName"
	status: ".Status"
	statusExtended: ".StatusExtended"
	severity: ".Severity"
	resourceId: ".ResourceId"
	resourceArn: ".ResourceArn"
	resourceTags: ".ResourceTags.{}"
	resourceType: ".ResourceType"
	resourceDetails: ".ResourceDetails"
	description: ".Description"
	risk: ".Risk"
	relatedUrl: ".RelatedUrl"
	recommendation: ".Remediation.Recommendation.Text"
	recommendationUrl: ".Remediation.Recommendation.Url"
	remediationCode: ".Remediation.Code.{}"
	categories: ".Categories.[]"
	notes: ".Notes"
	compliance: ".Compliance.{}"
ocsf:
	assessmentStartTime: ".event_time" # Format: "%Y-%m-%dT%H:%M:%S"
	findingUniqueId: ".finding_info.uid"
	provider: ".cloud.provider"
	profile: null
	accountId: ".cloud.account.uid"
	organizationsInfo: # .cloud.account.name, .cloud.org.name, .cloud.account.labels.[], .cloud.account.labels.{}
	region: ".resources[0].region"
	checkId: ".metadata.event_code"
	checkTitle: ".finding_info.title"
	checkTypes: ".finding_info.types.[]"
	serviceName: ".resources[0].group.name"
	subServiceName: null
	status: ".status_code"
	statusExtended: ".status_detail"
	severity: ".severity" # .severity_id could also map
	resourceId: ".resources[0].name"
	resourceArn: ".resources[0].uid"
	resourceTags: ".resources[0].labels.{}"
	resourceType: ".resources[0].type"
	resourceDetails: ".resources[0].data.details"
	description: ".finding_info.desc"
	risk: ".risk_details"
	relatedUrl: ".unmapped.related_url"
	recommendation: ".remediation.desc"
	recommendationUrl: ".remediation.references.[]" # Have to filter by http
	remediationCode: ".remediation.references.[]" # Have to filter by !http
	categories: ".unmapped.categories.[]"
	notes: ".unmapped.notes"
	compliance: ".unmapped.compliance.{[]}" # Might need special handling
```

#Pseudocode
```python
config = load_yaml()
data = load_data()
version = cmd_line_arguments().version
config = config[version]

def recursive_look(pattern, object):
  # Probably should do this elsewhere
  if type(pattern) is str:
    pattern = pattern.split('.')
  # Actual lookup
  if len(pattern) == 0:
    return object
  p = pattern[0]
  if p == '{}':
    return '\n'.join(f'{key}: {value}' for key, value in object.items())
  elif p == '[]':
    return '\n'.join(object)
  elif p == '{[]}':
    return '\n'.join(f'{key}: {"\n-".join(values)}' for key, values in object.items())
  else:
    recursive_look(pattern[1:], object[p])

ProwlerItem(
  assessmentStartTime=recursive_look(config['assessmentStartTime'], object),
  ...
  provider=recursive_look(config['provider'], object),
  ...
  provider=recursive_look(config['compliance'], object)
)
```
